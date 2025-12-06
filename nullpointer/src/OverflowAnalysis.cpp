#include "OverflowAnalysis.h"
#include "DomainOverflow.h"
#include "Utils.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"

using namespace llvm;
using overflow::DomainOverflow;

namespace dataflow {

std::vector<Instruction *> getPredecessors(Instruction *Inst);
std::vector<Instruction *> getSuccessors(Instruction *Inst);

// ===----------------------------------------------------------------------===//
// Local helpers (overflow-specific)
// ===----------------------------------------------------------------------===//

namespace {

// Use the same naming scheme as other analyses.
using dataflow::OverflowMemory;
using dataflow::variable;

// Get a domain element for Val from Mem, or synthesize it.
//
//  - If Mem has an entry for variable(Val), return it.
//  - If Val is a ConstantInt, return [v, v].
//  - Otherwise, return TOP ([-inf, +inf]).
DomainOverflow getOrExtractOverflow(const OverflowMemory &Mem,
                                    const Value *Val) {
  std::string name = variable(Val);
  auto It = Mem.find(name);
  if (It != Mem.end())
    return It->second;

  if (auto *CI = dyn_cast<ConstantInt>(Val)) {
    // For small bit widths (8-bit, 16-bit), we need to consider both
    // signed and unsigned interpretations
    unsigned BW = CI->getBitWidth();
    long long sval = CI->getSExtValue();  // Signed interpretation

    // For operations without explicit signed flags, use a conservative
    // interval that covers both signed and unsigned interpretations
    if (BW <= 16 && BW < 64) {
      // Get unsigned interpretation
      uint64_t uval = CI->getZExtValue();
      long long uval_signed = (long long)uval;

      // Return interval that covers both interpretations
      long long low = std::min(sval, uval_signed);
      long long high = std::max(sval, uval_signed);
      return DomainOverflow(low, high);
    }

    return DomainOverflow(sval, sval);
  }

  // Unknown value => top interval
  return DomainOverflow::top();
}

// Compare two memories for equality using DomainOverflow::equal.
bool memoryEqual(const OverflowMemory &A, const OverflowMemory &B) {
  if (A.size() != B.size())
    return false;

  for (const auto &KV : A) {
    auto It = B.find(KV.first);
    if (It == B.end())
      return false;
    if (!DomainOverflow::equal(KV.second, It->second))
      return false;
  }
  return true;
}

// Optional pretty-print helpers (only used inside this file).
void printOverflowMemory(const OverflowMemory &Mem) {
  for (const auto &KV : Mem) {
    errs() << "  [ " << KV.first << " |-> ";
    KV.second.print(errs());
    errs() << " ]\n";
  }
}

// void printOverflowMap(Function &F,
//                       std::map<Instruction *, OverflowMemory *> &InMap,
//                       std::map<Instruction *, OverflowMemory *> &OutMap) {
//   for (auto &BB : F) {
//     for (auto &I : BB) {
//       Instruction *Inst = &I;
//       errs() << Inst << "\n";
//       errs() << "  IN:\n";
//       printOverflowMemory(*InMap[Inst]);
//       errs() << "  OUT:\n";
//       printOverflowMemory(*OutMap[Inst]);
//       errs() << "\n";
//     }
//   }
// }

} // end anonymous namespace

// ===----------------------------------------------------------------------===//
// Transfer function
// ===----------------------------------------------------------------------===//
//
// NOut starts as a copy of In. Then we update it based on Inst.
//
// We only track integer-producing instructions (binary ops, PHI).
// Other instructions just propagate the input state unchanged.
//

void OverflowAnalysis::transfer(Instruction *I,
                                const OverflowMemory *In,
                                OverflowMemory &NOut) {
  // Start from incoming memory
  NOut = *In;

  // Binary integer operations
  if (auto *BO = dyn_cast<BinaryOperator>(I)) {
    if (!BO->getType()->isIntegerTy())
      return;

    DomainOverflow L = getOrExtractOverflow(*In, BO->getOperand(0));
    DomainOverflow R = getOrExtractOverflow(*In, BO->getOperand(1));
    DomainOverflow Res;

    switch (BO->getOpcode()) {
    case Instruction::Add:
      Res = DomainOverflow::add(L, R);
      break;
    case Instruction::Sub:
      Res = DomainOverflow::sub(L, R);
      break;
    case Instruction::Mul:
      Res = DomainOverflow::mul(L, R);
      break;
    case Instruction::Shl:
      Res = DomainOverflow::shl(L, R);
      break;
    default:
      // Other binary ops are ignored for this analysis
      return;
    }

    NOut[variable(I)] = Res;
    return;
  }

  // PHI nodes for integers: join intervals of incoming values
  if (auto *PN = dyn_cast<PHINode>(I)) {
    if (!PN->getType()->isIntegerTy())
      return;

    bool first = true;
    DomainOverflow Acc = DomainOverflow::bottom();

    for (unsigned i = 0, e = PN->getNumIncomingValues(); i < e; ++i) {
      Value *V = PN->getIncomingValue(i);
      DomainOverflow VDom = getOrExtractOverflow(*In, V);
      if (first) {
        Acc = VDom;
        first = false;
      } else {
        Acc = DomainOverflow::join(Acc, VDom);
      }
    }

    if (!first) // had at least one incoming value
      NOut[variable(I)] = Acc;

    return;
  }

  // Other instructions: nothing special, just propagate NOut = In.
}

// ===----------------------------------------------------------------------===//
// flowIn: meet over all predecessors
// ===----------------------------------------------------------------------===//

void OverflowAnalysis::flowIn(Instruction *Inst, OverflowMemory *InMem) {
  InMem->clear();

  // Instruction-level predecessors (previous instruction in block,
  // or terminators of predecessor blocks).
  std::vector<Instruction *> Preds = getPredecessors(Inst);

  // No predecessors (first instruction overall) → leave InMem empty.
  if (Preds.empty())
    return;

  bool firstPred = true;
  OverflowMemory Acc;

  for (Instruction *Pred : Preds) {
    OverflowMemory *PredOut = OutMap[Pred];

    if (firstPred) {
      // Initialize accumulator with predecessor OUT
      Acc = *PredOut;
      firstPred = false;
    } else {
      // Join Acc with PredOut element-wise.
      OverflowMemory NewAcc;

      // Keys from Acc
      for (const auto &KV : Acc) {
        const std::string &name = KV.first;
        auto It2 = PredOut->find(name);
        if (It2 != PredOut->end()) {
          NewAcc[name] = DomainOverflow::join(KV.second, It2->second);
        } else {
          NewAcc[name] = DomainOverflow::join(KV.second,
                                              DomainOverflow::top());
        }
      }

      // Keys only in PredOut
      for (const auto &KV : *PredOut) {
        const std::string &name = KV.first;
        if (Acc.find(name) == Acc.end()) {
          NewAcc[name] = DomainOverflow::join(DomainOverflow::top(),
                                              KV.second);
        }
      }

      Acc = std::move(NewAcc);
    }
  }

  *InMem = std::move(Acc);
}


// ===----------------------------------------------------------------------===//
// flowOut: update OutMap and workset
// ===----------------------------------------------------------------------===//
void OverflowAnalysis::flowOut(Instruction *Inst,
                               OverflowMemory *Pre,
                               OverflowMemory *Post,
                               SetVector<Instruction *> &WorkSet) {
  if (memoryEqual(*Pre, *Post))
    return;

  *Pre = *Post;

  // Re-enqueue successors if OUT changed.
  std::vector<Instruction *> Succs = getSuccessors(Inst);
  for (Instruction *Succ : Succs) {
    WorkSet.insert(Succ);
  }
}


// ===----------------------------------------------------------------------===//
// Chaotic iteration driver
// ===----------------------------------------------------------------------===//

void OverflowAnalysis::doAnalysis(Function &F) {
  SetVector<Instruction *> WorkSet;

  // Initialize workset with ALL instructions.
  for (Instruction &I : instructions(F)) {
    WorkSet.insert(&I);
  }

  while (!WorkSet.empty()) {
    Instruction *Inst = WorkSet.pop_back_val();

    OverflowMemory *InMem = InMap[Inst];
    OverflowMemory *OutMem = OutMap[Inst];

    // Compute IN
    flowIn(Inst, InMem);

    // Compute OUT via transfer
    OverflowMemory NewOut;
    transfer(Inst, InMem, NewOut);

    // Merge with previous OUT and update workset
    flowOut(Inst, OutMem, &NewOut, WorkSet);
  }
}

// ===----------------------------------------------------------------------===//
// Overflow check
// ===----------------------------------------------------------------------===//

bool OverflowAnalysis::check(Instruction *Inst) {
  auto *BO = dyn_cast<BinaryOperator>(Inst);
  if (!BO)
    return false;

  unsigned Opcode = BO->getOpcode();
  if (Opcode != Instruction::Add &&
      Opcode != Instruction::Sub &&
      Opcode != Instruction::Mul &&
      Opcode != Instruction::Shl)
    return false;

  Type *Ty = BO->getType();
  if (!Ty->isIntegerTy())
    return false;

  // Get the OUT interval for this instruction's result.
  OverflowMemory *OutMem = OutMap[Inst];
  DomainOverflow ResDom = getOrExtractOverflow(*OutMem, Inst);

  if (ResDom.isBottom)
    return false; // unreachable

  unsigned BW = Ty->getIntegerBitWidth();

  // Determine if we should check signed or unsigned bounds
  // Check for nsw (no signed wrap) flag - if present, it's a signed operation
  bool isSigned = false;
  if (BO->hasNoSignedWrap()) {
    isSigned = true;
  } else if (BO->hasNoUnsignedWrap()) {
    isSigned = false;
  } else {
    // No flags - heuristic: treat as signed for compatibility
    // But check both signed and unsigned bounds
    isSigned = true;
  }

  long long minVal, maxVal;

  if (isSigned) {
    // Signed bounds: [-2^(BW-1), 2^(BW-1) - 1]
    minVal = -(1LL << (BW - 1));
    maxVal =  (1LL << (BW - 1)) - 1;
  } else {
    // Unsigned bounds: [0, 2^BW - 1]
    minVal = 0;
    maxVal = (1LL << BW) - 1;
  }

  // If mathematical interval escapes machine range, may overflow.
  if (ResDom.low < minVal || ResDom.high > maxVal)
    return true;

  // For operations without flags, also check if it would overflow
  // if interpreted as unsigned (common for small bit widths)
  if (!BO->hasNoSignedWrap() && !BO->hasNoUnsignedWrap() && BW <= 16) {
    // Check unsigned bounds
    long long unsignedMin = 0;
    long long unsignedMax = (BW < 63) ? ((1LL << BW) - 1) : LLONG_MAX;

    // Only flag if it overflows in BOTH interpretations, OR
    // if the interval clearly exceeds unsigned range
    if (ResDom.high > unsignedMax)
      return true;
  }

  return false;
}

// ===----------------------------------------------------------------------===//
// Pass entry point
// ===----------------------------------------------------------------------===//

PreservedAnalyses OverflowAnalysis::run(Function &F,
                                        FunctionAnalysisManager &) {
  outs() << "Running " << getAnalysisName() << " on " << F.getName() << "\n";

  // Initialize InMap and OutMap.
  for (inst_iterator It = inst_begin(F), End = inst_end(F); It != End; ++It) {
    Instruction *Inst = &*It;
    InMap[Inst]  = new OverflowMemory;
    OutMap[Inst] = new OverflowMemory;
  }

  // Chaotic iteration.
  doAnalysis(F);

  // Check each instruction for possible overflow.
  for (inst_iterator It = inst_begin(F), End = inst_end(F); It != End; ++It) {
    Instruction *Inst = &*It;
    if (check(Inst))
      ErrorInsts.insert(Inst);
  }

  // Optional: print the analysis result
  // printOverflowMap(F, InMap, OutMap);

  outs() << "Potential Overflow Instructions by " << getAnalysisName() << ":\n";
  for (auto *Inst : ErrorInsts) {
    outs() << *Inst << "\n";
  }

  // Cleanup
  for (inst_iterator It = inst_begin(F), End = inst_end(F); It != End; ++It) {
    Instruction *Inst = &*It;
    delete InMap[Inst];
    delete OutMap[Inst];
  }

  return PreservedAnalyses::all();
}

// ===----------------------------------------------------------------------===//
// Pass registration
// ===----------------------------------------------------------------------===//

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Overflow", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "Overflow") {
                    MPM.addPass(
                        createModuleToFunctionPassAdaptor(OverflowAnalysis()));
                    return true;
                  }
                  return false;
                });
          }};
}

} // namespace dataflow