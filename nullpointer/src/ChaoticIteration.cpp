#include "NullPointerAnalysis.h"
#include "Utils.h"

namespace dataflow {

/**
 * @brief Get the Predecessors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the predecessors of.
 * @return Vector of all predecessors of Inst.
 */
std::vector<Instruction *> getPredecessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->rbegin(), End = Block->rend(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Pre = pred_begin(Block), BE = pred_end(Block); Pre != BE; ++Pre) {
        Ret.push_back(&(*((*Pre)->rbegin())));
      }
      return Ret;
    }
  }
  return Ret;
}

/**
 * @brief Get the successors of a given instruction in the control-flow graph.
 *
 * @param Inst The instruction to get the successors of.
 * @return Vector of all successors of Inst.
 */
std::vector<Instruction *> getSuccessors(Instruction *Inst) {
  std::vector<Instruction *> Ret;
  auto Block = Inst->getParent();
  for (auto Iter = Block->begin(), End = Block->end(); Iter != End; ++Iter) {
    if (&(*Iter) == Inst) {
      ++Iter;
      if (Iter != End) {
        Ret.push_back(&(*Iter));
        return Ret;
      }
      for (auto Succ = succ_begin(Block), BS = succ_end(Block); Succ != BS; ++Succ) {
        Ret.push_back(&(*((*Succ)->begin())));
      }
      return Ret;
    }
  }
  return Ret;
}

/**
 * @brief Joins two Memory objects (Mem1 and Mem2), accounting for Domain
 * values.
 *
 * @param Mem1 First memory.
 * @param Mem2 Second memory.
 * @return The joined memory.
 */
Memory *join(Memory *Mem1, Memory *Mem2) {
  /**
   * TODO: Write your code that joins two memories.
   *
   * If some instruction with domain D is either in Mem1 or Mem2, but not in
   *   both, add it with domain D to the Result.
   * If some instruction is present in Mem1 with domain D1 and in Mem2 with
   *   domain D2, then Domain::join D1 and D2 to find the new domain D,
   *   and add instruction I with domain D to the Result.
   */
  auto *Result = new Memory();

  std::set<std::string> Keys;
  for (auto &P : *Mem1)
    Keys.insert(P.first);
  for (auto &P : *Mem2)
    Keys.insert(P.first);

  for (const auto &Key : Keys) {
    auto It1 = Mem1->find(Key);
    auto It2 = Mem2->find(Key);

    Domain *JoinedDomain = nullptr;

    if (It1 != Mem1->end() && It2 != Mem2->end()) {
      JoinedDomain = Domain::join(It1->second, It2->second);
    } else if (It1 != Mem1->end()) {
      JoinedDomain = new Domain(*(It1->second));
    } else if (It2 != Mem2->end()) {
      JoinedDomain = new Domain(*(It2->second));
    }

    (*Result)[Key] = JoinedDomain;
  }
  return Result;
}

// Helper to deep copy a Memory map
Memory* cloneMemory(const Memory* Src) {
    Memory* Dst = new Memory();
    for (auto const& [Key, Val] : *Src) {
        (*Dst)[Key] = new Domain(*Val);
    }
    return Dst;
}

/** Refines domain based on condition. Returns false if branch is unreachable. */
bool refine(Memory* Mem, Value* Cond, bool isTrueBranch) {
    auto* Cmp = dyn_cast<ICmpInst>(Cond);
    if (!Cmp) return true; 

    Value* Op0 = Cmp->getOperand(0);
    Value* Op1 = Cmp->getOperand(1);
    auto Pred = Cmp->getPredicate();

    // Make sure Op1 is the NULL
    if (isa<ConstantPointerNull>(Op0)) {
        std::swap(Op0, Op1);
        Pred = Cmp->getSwappedPredicate();
    }

    if (!isa<ConstantPointerNull>(Op1)) return true;

    // Determine Required Domain
    Domain::Element Required;
    if (Pred == CmpInst::ICMP_EQ) {
        Required = isTrueBranch ? Domain::Null : Domain::NonNull;
    } else if (Pred == CmpInst::ICMP_NE) {
        Required = isTrueBranch ? Domain::NonNull : Domain::Null;
    } else {
        return true; 
    }

    // Function to update a variable in memory
    auto updateMemory = [&](std::string Name) -> bool {
        if (Mem->count(Name)) {
            Domain* Current = (*Mem)[Name];
            if (Current->Value == Domain::MaybeNull) {
                // Refine MaybeNull -> Required domain
                delete Current;
                (*Mem)[Name] = new Domain(Required);
            } 
            else if (Current->Value != Required) {
                // Contradiction
                return false;
            }
        } else {
            // If not in map, assume it was unknown/uninit, set to Required
            (*Mem)[Name] = new Domain(Required);
        }
        return true;
    };

    // Refine the operand
    if (!updateMemory(variable(Op0))) return false;

    // If Op0 is a load, also refine the source memory variable
    if (auto *Load = dyn_cast<LoadInst>(Op0)) {
        Value *SourcePtr = Load->getPointerOperand()->stripPointerCasts();
        
        if (isa<AllocaInst>(SourcePtr)) {
            if (!updateMemory(variable(SourcePtr))) return false;
        }
    }

    return true;
}

bool NullPointerAnalysis::flowIn(Instruction *Inst, Memory *InMem) {
  std::vector<Instruction *> Preds = getPredecessors(Inst);

  if (Inst == &(Inst->getFunction()->getEntryBlock().front())) {
    for (Argument &Arg : Inst->getFunction()->args()) {
      // Assume arguments can be any value, so they are MaybeNull.
      (*InMem)[variable(&Arg)] = new Domain(Domain::MaybeNull);
    }
    return true;
  }

  bool atLeastOnePath = false; // Track if we found a valid path
  bool firstMerge = true;

  for (Instruction *Pred : Preds) {
    if (OutMap.find(Pred) == OutMap.end()) continue;

    Memory *PredOut = OutMap[Pred];
    Memory *EdgeMem = cloneMemory(PredOut); // Work on a copy
    bool isFeasible = true;

    // errs() << "\n[FlowIn] Processing Edge: " << variable(Pred) << " -> " << variable(Inst) << "\n";

    // Check if the predecessor is a conditional branch
    if (auto *Branch = dyn_cast<BranchInst>(Pred)) {
      if (Branch->isConditional()) {
        Value *Cond = Branch->getCondition();
        BasicBlock *CurrentBlock = Inst->getParent();

        // Check True Edge
        if (Branch->getSuccessor(0) == CurrentBlock) {
           if (!refine(EdgeMem, Cond, true)) {
               isFeasible = false;
           }
        }
        // Check False Edge
        else if (Branch->getSuccessor(1) == CurrentBlock) {
           if (!refine(EdgeMem, Cond, false)) {
               isFeasible = false;
           }
        }
      }
    }

    // Only join if the path is feasible
    if (isFeasible) {
        atLeastOnePath = true;
        if (firstMerge) {
            // Copy EdgeMem to InMem directly for the first valid predecessor
            for (auto const& [Key, Val] : *EdgeMem) {
                (*InMem)[Key] = new Domain(*Val);
            }
            firstMerge = false;
        } else {
            // Join with accumulated InMem
            Memory *Joined = join(InMem, EdgeMem);
            
            for (auto &pair : *InMem) delete pair.second;
            InMem->clear();

            // Update InMem
            for (auto const& [Key, Val] : *Joined) {
                (*InMem)[Key] = new Domain(*Val);
            }
            delete Joined;
        }
    }

    // Clean up the temporary edge memory
    for (auto &pair : *EdgeMem) delete pair.second;
    delete EdgeMem;
  }

  return atLeastOnePath;
}

/**
 * @brief This function returns true if the two memories Mem1 and Mem2 are
 * equal.
 *
 * @param Mem1 First memory
 * @param Mem2 Second memory
 * @return true if the two memories are equal, false otherwise.
 */
bool equal(Memory *Mem1, Memory *Mem2) {
  /**
   * TODO: Write your code to implement check for equality of two memories.
   *
   * If any instruction I is present in one of Mem1 or Mem2,
   *   but not in both and the Domain of I is not UnInit, the memories are
   *   unequal.
   * If any instruction I is present in Mem1 with domain D1 and in Mem2
   *   with domain D2, if D1 and D2 are unequal, then the memories are unequal.
   */

  // Get all instructions (keys)
  std::set<std::string> Keys;
  for (auto &P : *Mem1)
    Keys.insert(P.first);
  for (auto &P : *Mem2)
    Keys.insert(P.first);

  for (const auto &Key : Keys) {
    auto It1 = Mem1->find(Key);
    auto It2 = Mem2->find(Key);

    bool In1 = (It1 != Mem1->end());
    bool In2 = (It2 != Mem2->end());

    //  Key appears in only one map
    if (In1 && !In2) {
      if (!Domain::equal(*(It1->second), Domain::Uninit))
        return false;
    } else if (!In1 && In2) {
      if (!Domain::equal(*(It2->second), Domain::Uninit))
        return false;
    }

    // Appears in both maps
    else if (In1 && In2) {
      if (!Domain::equal(*(It1->second), *(It2->second)))
        return false;
    }
  }

  return true;
}

void NullPointerAnalysis::flowOut(
    Instruction *Inst, Memory *Pre, Memory *Post, SetVector<Instruction *> &WorkSet) {
  /**
   * TODO: Write your code to implement flowOut.
   *
   * For each given instruction, merge abstract domain from pre-transfer memory
   * and post-transfer memory, and update the OutMap.
   * If the OutMap changed then also update the WorkSet.
   */

  if (!equal(Pre, Post)) {
    *Pre = *Post;
    for (Instruction *Succ : getSuccessors(Inst))
      WorkSet.insert(Succ);
  }

}

void NullPointerAnalysis::doAnalysis(Function &F, PointerAnalysis *PA) {
  SetVector<Instruction *> WorkSet;
  SetVector<Value *> PointerSet;
  /**
   * TODO: Write your code to implement the chaotic iteration algorithm
   * for the analysis.
   *
   * Initialize the WorkSet with all the instructions in the function.
   *
   * While the WorkSet is not empty:
   * - Pop an instruction from the WorkSet.
   * - Construct it's Incoming Memory using flowIn.
   * - Evaluate the instruction using transfer and create the OutMemory.
   * - Use flowOut along with the previous Out memory and the current Out
   *   memory, to check if there is a difference between the two to update the
   *   OutMap and add all successors to WorkSet.
   */

  // Initialize PointerSet with all pointer-type values
  for (Instruction &I : instructions(F)) {
    if (I.getType()->isPointerTy()) {
     PointerSet.insert(&I);
   }
    // Also include operands that are pointers
    for (Use &U : I.operands()) {
      if (U->getType()->isPointerTy()) {
        PointerSet.insert(U.get());
     }
    }
  }

  // Create initial memory for entry instruction
  Memory *EntryMem = new Memory();
  for (Argument &Arg : F.args()) {
    std::string Var = variable(&Arg);
    if (Arg.getType()->isIntegerTy()) {
      (*EntryMem)[Var] = new Domain(Domain::MaybeNull);
    } else if (Arg.getType()->isPointerTy()) {
      PointerSet.insert(&Arg);
    }
  }

  // Associate this memory with the first instruction of the entry block
  Instruction *FirstInst = &*F.getEntryBlock().begin();
  InMap[FirstInst] = EntryMem;

  // Initialize workset
  for (Instruction &I : instructions(F)) {
    WorkSet.insert(&I);
  }


  while (!WorkSet.empty()) {
    Instruction *Inst = WorkSet.pop_back_val();
    // errs() << "Processing instruction: " << *Inst << "\n";

    Memory *InMem = new Memory();

    bool isReachable = flowIn(Inst, InMem);
    InMap[Inst] = InMem;
    // errs() << "InMap after flowIn for: " << *Inst << "\n";
    // InMap[Inst] = new Memory(InMem);

    if (!isReachable) {
      Memory *OldOut = OutMap[Inst];
      if (!OldOut->empty()) {
          OldOut->clear(); // Set to Bottom
          for (Instruction *Succ : getSuccessors(Inst)) {
              WorkSet.insert(Succ);
          }
      }
      continue;
    }
    Memory *Out = new Memory();

    // Copy InMem into Out
    for (auto const &[key, val] : *InMem) {
      (*Out)[key] = new Domain(*val);
    }

    NullPointerAnalysis::transfer(Inst, InMem, *Out, PA, PointerSet);
    // printInstructionTransfer(Inst, InMem, Out);
    flowOut(Inst, OutMap[Inst], Out, WorkSet);

  }
}

}  // namespace dataflow
