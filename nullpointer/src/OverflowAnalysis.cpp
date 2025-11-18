#include "OverflowAnalysis.h"

#include "Utils.h"

namespace dataflow {

//===----------------------------------------------------------------------===//
// Overflow Analysis Implementation
//===----------------------------------------------------------------------===//

bool OverflowAnalysis::check(Instruction *Inst) {
  // Only checking for integer ops
  if (!(isa<BinaryOperator>(Inst) &&
        (Inst->getOpcode() == Instruction::Add ||
         Inst->getOpcode() == Instruction::Sub ||
         Inst->getOpcode() == Instruction::Mul ||
         Inst->getOpcode() == Instruction::Shl ||
        )
    )) {
    return false;
  }

  Value *Divisor = Inst->getOperand(1);
  Domain *DivisorDomain = getOrExtract(InMap[Inst], Divisor);

  return (Domain::equal(*DivisorDomain, Domain::Zero) || Domain::equal(*DivisorDomain, Domain::MaybeZero));
}

PreservedAnalyses DivZeroAnalysis::run(Function &F, FunctionAnalysisManager &) {
  outs() << "Running " << getAnalysisName() << " on " << F.getName() << "\n";

  // Initializing InMap and OutMap.
  for (inst_iterator Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    auto Inst = &(*Iter);
    InMap[Inst] = new Memory;
    OutMap[Inst] = new Memory;
  }

  // The chaotic iteration algorithm is implemented inside doAnalysis().
  doAnalysis(F);

  // Check each instruction in function F for potential divide-by-zero error.
  for (inst_iterator Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    auto Inst = &(*Iter);
    if (check(Inst))
      ErrorInsts.insert(Inst);
  }

  printMap(F, InMap, OutMap);
  outs() << "Potential Instructions by " << getAnalysisName() << ": \n";
  for (auto Inst : ErrorInsts) {
    outs() << *Inst << "\n";
  }

  for (auto Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    delete InMap[&(*Iter)];
    delete OutMap[&(*Iter)];
  }
  return PreservedAnalyses::all();
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivZero", "v0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                    ModulePassManager &MPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "DivZero") {
                    MPM.addPass(createModuleToFunctionPassAdaptor(DivZeroAnalysis()));
                    return true;
                  }
                  return false;
                });
          }};
}
}  // namespace dataflow
