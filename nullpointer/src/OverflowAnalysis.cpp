#include "OverflowAnalysis.h"

#include "Utils.h"

namespace dataflow {

//===----------------------------------------------------------------------===//
// Overflow Analysis Implementation
//===----------------------------------------------------------------------===//

bool OverflowAnalysis::check(Instruction *Inst) {
  // Only checking for integer ops that can overflow
  if (!(isa<BinaryOperator>(Inst) &&
        (Inst->getOpcode() == Instruction::Add ||
         Inst->getOpcode() == Instruction::Sub ||
         Inst->getOpcode() == Instruction::Mul ||
         Inst->getOpcode() == Instruction::Shl))) {
    return false;
  }

  Memory *OutMem = OutMap[Inst];
  Domain *ResDomain = getOrExtract(OutMem, Inst);

  return (Domain::equal(*ResDomain, Domain::Overflow) || Domain::equal(*ResDomain, Domain::MaybeOverflow));
}

PreservedAnalyses OverflowAnalysis::run(Function &F, FunctionAnalysisManager &) {
  outs() << "Running " << getAnalysisName() << " on " << F.getName() << "\n";

  // Initializing InMap and OutMap.
  for (inst_iterator Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    auto Inst = &(*Iter);
    InMap[Inst] = new Memory;
    OutMap[Inst] = new Memory;
  }

  // The chaotic iteration algorithm is implemented inside doAnalysis().
  doAnalysis(F);

  // Check each instruction in function F for potential overflow error.
  for (inst_iterator Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    auto Inst = &(*Iter);
    if (check(Inst))
      ErrorInsts.insert(Inst);
  }

  printMap(F, InMap, OutMap);
  outs() << "Potential Overflow Instructions by " << getAnalysisName() << ": \n";
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
  return {LLVM_PLUGIN_API_VERSION, "Overflow", "v0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                    ModulePassManager &MPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "Overflow") {
                    MPM.addPass(createModuleToFunctionPassAdaptor(OverflowAnalysis()));
                    return true;
                  }
                  return false;
                });
          }};
}
}  // namespace dataflow
