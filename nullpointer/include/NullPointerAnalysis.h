#ifndef NULL_POINTER_ANALYSIS_H
#define NULL_POINTER_ANALYSIS_H

#include "Domain.h"
#include "PointerAnalysis.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <string>

namespace dataflow {

using Memory = std::map<std::string, Domain *>;

struct NullPointerAnalysis : public llvm::PassInfoMixin<NullPointerAnalysis> {
  std::map<llvm::Instruction *, Memory *> InMap;
  std::map<llvm::Instruction *, Memory *> OutMap;
  llvm::SetVector<llvm::Instruction *> ErrorInsts;

  /**
   * This function is called for each function F in the input C program
   * that the compiler encounters during a pass.
   * You do not need to modify this function.
   */
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &);

 protected:
  /**
   * This function creates a transfer function that updates the Out Memory based
   * on In Memory and the instruction type/parameters.
   */
  void transfer(Instruction *I,
      const Memory *In,
      Memory &NOut,
      PointerAnalysis *PA,
      SetVector<Value *> PointerSet);

  /**
   * @brief This function implements the chaotic iteration algorithm using
   * flowIn(), transfer(), and flowOut().
   *
   * @param F The function to be analyzed.
   */
  void doAnalysis(Function &F, PointerAnalysis *PA);

  /**
   * @brief Flow the abstract domains from all predecessors of Inst into the In
   * Memory object for Inst.
   *
   * @param Inst Instruction to flow In Memory for.
   * @param InMem InMemory object of Inst to populate.
   */
  void flowIn(Instruction *Inst, Memory *InMem);

  /**
   * @brief Merge the previous Out Memory of Inst with the current Out Memory
   * for each instruction to update the OutMap and WorkSet as needed.
   *
   * @param Inst Instruction to flow Out Memory for.
   * @param Pre Previous OutMemory of Inst.
   * @param Post Current OutMemory of Inst.
   * @param WorkSet WorkSet
   */
  void flowOut(
      Instruction *Inst, Memory *Pre, Memory *Post, SetVector<Instruction *> &WorkSet);

  /**
   * Can the Instruction Inst incurr a null pointer dereference error?
   *
   * @param Inst Instruction to check.
   * @return true if the instruction can incur a null pointer dereference error.
   */
  bool check(Instruction *Inst);

  std::string getAnalysisName() {
    return "NullPtr";
  }
};
}  // namespace dataflow

#endif  // NULL_POINTER_ANALYSIS_H
