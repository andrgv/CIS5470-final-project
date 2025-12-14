#include "NullPointerAnalysis.h"
#include "OverflowAnalysis.h"
#include "Utils.h"

namespace dataflow {

/**
 * @brief Is the given instruction a user input?
 *
 * @param Inst The instruction to check.
 * @return true If it is a user input, false otherwise.
 */
bool isInput(Instruction *Inst) {
  if (auto Call = dyn_cast<CallInst>(Inst)) {
    if (auto Fun = Call->getCalledFunction()) {
      return (Fun->getName().equals("getchar") || Fun->getName().equals("fgetc"));
    }
  }
  return false;
}

/**
 * Evaluate a PHINode to get its Domain.
 *
 * @param Phi PHINode to evaluate
 * @param InMem InMemory of Phi
 * @return Domain of Phi
 */
Domain *eval(PHINode *Phi, const Memory *InMem) {
  if (auto ConstantVal = Phi->hasConstantValue()) {
    return new Domain(extractFromValue(ConstantVal));
  }

  Domain *Joined = new Domain(Domain::Uninit);

  for (unsigned int i = 0; i < Phi->getNumIncomingValues(); i++) {
    auto Dom = getOrExtract(InMem, Phi->getIncomingValue(i));
    Joined = Domain::join(Joined, Dom);
  }
  return Joined;
}

/**
 * @brief Evaluate the +, -, * and / BinaryOperator instructions
 * using the Domain of its operands and return the Domain of the result.
 *
 * @param BinOp BinaryOperator to evaluate
 * @param InMem InMemory of BinOp
 * @return Domain of BinOp
 */
Domain *eval(BinaryOperator *BinOp, const Memory *InMem) {
  /**
   * TODO: Write your code here that evaluates +, -, * and /
   * based on the Domains of the operands.
   */
  Value *LHS = BinOp->getOperand(0);
  Value *RHS = BinOp->getOperand(1);

  Domain *Left = getOrExtract(InMem, LHS);
  Domain *Right = getOrExtract(InMem, RHS);

  if (BinOp->getOpcode() == Instruction::Add) {
    return Domain::add(Left, Right);
  } else if (BinOp->getOpcode() == Instruction::Sub) {
    return Domain::sub(Left, Right);
  } else if (BinOp->getOpcode() == Instruction::Mul) {
    return Domain::mul(Left, Right);
  } else if (BinOp->getOpcode() == Instruction::SDiv || BinOp->getOpcode() == Instruction::UDiv) {
    return Domain::div(Left, Right);
  }
  // For other binary operators, return Uninit
  return new Domain(Domain::Uninit);
}

/**
 * @brief Evaluate Cast instructions.
 *
 * @param Cast Cast instruction to evaluate
 * @param InMem InMemory of Instruction
 * @return Domain of Cast
 */
Domain *eval(CastInst *Cast, const Memory *InMem) {
  /**
   * TODO: Write your code here to evaluate Cast instruction.
   */
  Value *Operand = Cast->getOperand(0);
  Domain *OpDomain = getOrExtract(InMem, Operand);

  return new Domain(*OpDomain);
}

/**
 * @brief Evaluate the ==, !=, <, <=, >=, and > Comparision operators using
 * the Domain of its operands to compute the Domain of the result.
 *
 * @param Cmp Comparision instruction to evaluate
 * @param InMem InMemory of Cmp
 * @return Domain of Cmp
 */
// Domain *eval(CmpInst *Cmp, const Memory *InMem) {
//   /**
//    * TODO: Write your code here that evaluates:
//    * ==, !=, <, <=, >=, and > based on the Domains of the operands.
//    *
//    * NOTE: There is a lot of scope for refining this, but you can just return
//    * MaybeZero for comparisons other than equality.
//    */
//   Value *LHS = Cmp->getOperand(0);
//   Value *RHS = Cmp->getOperand(1);

//   Domain *Left = getOrExtract(InMem, LHS);
//   Domain *Right = getOrExtract(InMem, RHS);

//   auto Pred = Cmp->getPredicate();

//   if (Pred == CmpInst::ICMP_EQ) {
//     // Equal (true)
//     if (Domain::equal(*Left, Domain::Zero) && Domain::equal(*Right, Domain::Zero)) {
//       return new Domain(Domain::NonZero);
//     }

//     // Not equal (false)
//     if ((Domain::equal(*Left, Domain::Zero) && Domain::equal(*Right, Domain::NonZero)) ||
//         (Domain::equal(*Left, Domain::NonZero) && Domain::equal(*Right, Domain::Zero))) {
//       return new Domain(Domain::Zero);
//     }

//     return new Domain(Domain::MaybeZero);

//   } else if (Pred == CmpInst::ICMP_NE) {
//     // Equal (false)
//     if (Domain::equal(*Left, Domain::Zero) && Domain::equal(*Right, Domain::Zero)) {
//       return new Domain(Domain::Zero);
//     }

//     // Not equal (true)
//     if ((Domain::equal(*Left, Domain::Zero) && Domain::equal(*Right, Domain::NonZero)) ||
//         (Domain::equal(*Left, Domain::NonZero) && Domain::equal(*Right, Domain::Zero))) {
//       return new Domain(Domain::NonZero);
//     }

//     return new Domain(Domain::MaybeZero);
//   }
  
//   return new Domain(Domain::MaybeZero); // Other comparisons (e.g., <, <=, >, >=)
// }

void NullPointerAnalysis::transfer(Instruction *Inst,
    const Memory *In,
    Memory &NOut,
    PointerAnalysis *PA,
    SetVector<Value *> PointerSet) {
  if (auto Phi = dyn_cast<PHINode>(Inst)) {
    // Evaluate PHI node
    NOut[variable(Phi)] = eval(Phi, In);
  } else if (auto BinOp = dyn_cast<BinaryOperator>(Inst)) {
    // Evaluate BinaryOperator
    // NOut[variable(BinOp)] = eval(BinOp, In);
  } else if (auto Cast = dyn_cast<CastInst>(Inst)) {
    // Evaluate Cast instruction
    NOut[variable(Cast)] = eval(Cast, In);
  } else if (auto Cmp = dyn_cast<CmpInst>(Inst)) {
    // Evaluate Comparision instruction
    // NOut[variable(Cmp)] = eval(Cmp, In);
  } else if (auto Alloca = dyn_cast<AllocaInst>(Inst)) {
    NOut[variable(Alloca)] = new Domain(Domain::NonNull);
    // NOut[variable(Alloca)] = new Domain(Domain::MaybeNull); // TODO: changed (test)
  } else if (auto Store = dyn_cast<StoreInst>(Inst)) {

    auto *Ptr = Store->getPointerOperand();
    auto *Val = Store->getValueOperand();

    if (!Val->getType()->isPointerTy()) return;

    std::string PtrName = variable(Ptr);
    
    // Allocas are nonnull
    Domain *ValDom = nullptr;
    if (isa<AllocaInst>(Val->stripPointerCasts())) {
        ValDom = new Domain(Domain::NonNull);
    } else {
        ValDom = getOrExtract(In, Val);
    }

    // Identify all aliases
    std::vector<std::string> Aliases;

    for (auto *P : PointerSet) {
      if (isa<AllocaInst>(P)) {
        std::string AliasName = variable(P);
        if (PA->alias(PtrName, AliasName)) {
           Aliases.push_back(AliasName);
        }
      }
    }

    if (Aliases.size() == 1) {
       // Directly assign if only 1 alias
       NOut[Aliases[0]] = ValDom;
    } else {
       // Join with old values
       for (const auto &Alias : Aliases) {
          Domain *OldVal = In->count(Alias) ? In->at(Alias) : new Domain(Domain::Uninit);
          NOut[Alias] = Domain::join(OldVal, ValDom);
       }
    }

  } else if (auto Load = dyn_cast<LoadInst>(Inst)) {

    if (!Load->getType()->isPointerTy()) return;

    Value *Ptr = Load->getPointerOperand();
    std::string PtrName = variable(Ptr);
    std::string DestName = variable(Load);

    Domain *Loaded = new Domain(Domain::Uninit);

    // Join domain values from all aliases
    for (auto *P : PointerSet) {
        if (isa<AllocaInst>(P)) {
            std::string Key = variable(P);
            if (PA->alias(PtrName, Key)) {
                if (In->count(Key)) {
                     Loaded = Domain::join(Loaded, In->at(Key));
                }
            }
        }
    }
    
    // Fallback if Uninit
    if (Domain::equal(*Loaded, Domain::Uninit)) {
        NOut[DestName] = new Domain(Domain::MaybeNull);
    } else {
        NOut[DestName] = Loaded;
    }

  } else if (auto Branch = dyn_cast<BranchInst>(Inst)) {
    // Analysis is flow-insensitive, so do nothing here.
  } else if (auto Call = dyn_cast<CallInst>(Inst)) {
    // Analysis is intra-procedural, so do nothing here.
  } else if (auto Return = dyn_cast<ReturnInst>(Inst)) {
    // Analysis is intra-procedural, so do nothing here.
  } else {
    errs() << "Unhandled instruction: " << *Inst << "\n";
  }
}

// void OverflowAnalysis::transfer(Instruction *Inst, const Memory *In, Memory &NOut) {
//   if (isInput(Inst)) {
//     // The instruction is a user controlled input, it can have any value.
//     NOut[variable(Inst)] = new Domain(Domain::MaybeZero);
//   } else if (auto Phi = dyn_cast<PHINode>(Inst)) {
//     // Evaluate PHI node
//     NOut[variable(Phi)] = eval(Phi, In);
//   } else if (auto BinOp = dyn_cast<BinaryOperator>(Inst)) {
//     // Evaluate BinaryOperator
//     NOut[variable(BinOp)] = eval(BinOp, In);
//   } else if (auto Cast = dyn_cast<CastInst>(Inst)) {
//     // Evaluate Cast instruction
//     NOut[variable(Cast)] = eval(Cast, In);
//   } else if (auto Cmp = dyn_cast<CmpInst>(Inst)) {
//     // Evaluate Comparision instruction
//     NOut[variable(Cmp)] = eval(Cmp, In);
//   } else if (auto Alloca = dyn_cast<AllocaInst>(Inst)) {
//     NOut[variable(Alloca)] = new Domain(Domain::NotNull);
//   } else if (auto Store = dyn_cast<StoreInst>(Inst)) {
//     // Used for the next lab, do nothing here.
//   } else if (auto Load = dyn_cast<LoadInst>(Inst)) {
//     // Used for the next lab, do nothing here.
//   } else if (auto Branch = dyn_cast<BranchInst>(Inst)) {
//     // Analysis is flow-insensitive, so do nothing here.
//   } else if (auto Call = dyn_cast<CallInst>(Inst)) {
//     // Analysis is intra-procedural, so do nothing here.
//   } else if (auto Return = dyn_cast<ReturnInst>(Inst)) {
//     // Analysis is intra-procedural, so do nothing here.
//   } else {
//     errs() << "Unhandled instruction: " << *Inst << "\n";
//   }
// }

}  // namespace dataflow
