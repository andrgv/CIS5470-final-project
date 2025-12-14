#include "NullPointerAnalysis.h"

#include "Utils.h"
#include <iostream>

namespace dataflow {

//===----------------------------------------------------------------------===//
// Null Pointer Dereference Analysis Implementation
//===----------------------------------------------------------------------===//


bool NullPointerAnalysis::check(Instruction *Inst) {

  Value *Ptr = nullptr;

  if (auto Load = dyn_cast<LoadInst>(Inst)) {
    Ptr = Load->getPointerOperand();
  } else if (auto Store = dyn_cast<StoreInst>(Inst)) {
    Ptr = Store->getPointerOperand();
  } else if (auto GEP = dyn_cast<GetElementPtrInst>(Inst)) {
    Ptr = GEP->getPointerOperand();
  }

  if (!Ptr) return false;

  // Pointers to stack are safe
  if (isa<AllocaInst>(Ptr->stripPointerCasts())) {
      return false;
  }

  // Retrieve the domain of the pointer
  IntOverflowDomain *PtrDomain = getOrExtract(InMap[Inst], Ptr);

  // Error if the pointer is Null or MaybeNull
  return (Domain::equal(*PtrDomain, Domain::Null) || 
          Domain::equal(*PtrDomain, Domain::MaybeNull));
}

PreservedAnalyses NullPointerAnalysis::run(Function &F, FunctionAnalysisManager &) {
  outs() << "Running " << getAnalysisName() << " on " << F.getName() << "\n";

  // Initializing InMap and OutMap.
  for (inst_iterator Iter = inst_begin(F), End = inst_end(F); Iter != End; ++Iter) {
    auto Inst = &(*Iter);
    InMap[Inst] = new Memory;
    OutMap[Inst] = new Memory;
  }

  // The chaotic iteration algorithm is implemented inside doAnalysis().
  auto PA = new PointerAnalysis(F);
  doAnalysis(F, PA);

  // Check each instruction in function F for potential null pointer dereference error.
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
  return {LLVM_PLUGIN_API_VERSION, "NullPtr", "v0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                    ModulePassManager &MPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "NullPtr") {
                    MPM.addPass(createModuleToFunctionPassAdaptor(NullPointerAnalysis()));
                    return true;
                  }
                  return false;
                });
          }};
}
}  // namespace dataflow
