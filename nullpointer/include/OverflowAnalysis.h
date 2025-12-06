#ifndef OVERFLOW_ANALYSIS_H
#define OVERFLOW_ANALYSIS_H

#include "DomainOverflow.h"

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

// Interval analysis memory: map variable name -> interval (DomainOverflow)
using OverflowMemory = std::map<std::string, overflow::DomainOverflow>;

struct OverflowAnalysis : public llvm::PassInfoMixin<OverflowAnalysis> {
  // Dataflow state: IN and OUT memory per instruction
  std::map<llvm::Instruction *, OverflowMemory *> InMap;
  std::map<llvm::Instruction *, OverflowMemory *> OutMap;

  // Instructions that may overflow
  llvm::SetVector<llvm::Instruction *> ErrorInsts;

  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);

protected:
  // Transfer function: In -> NOut for a single instruction
  void transfer(llvm::Instruction *I,
                const OverflowMemory *In,
                OverflowMemory &NOut);

  // Chaotic iteration driver
  void doAnalysis(llvm::Function &F);

  // Flow IN: join predecessors' OUT into InMem
  void flowIn(llvm::Instruction *Inst, OverflowMemory *InMem);

  // Flow OUT: merge Pre and Post, update OutMap + workset
  void flowOut(llvm::Instruction *Inst,
               OverflowMemory *Pre,
               OverflowMemory *Post,
               llvm::SetVector<llvm::Instruction *> &WorkSet);

  // Can Inst incur an integer overflow or underflow?
  bool check(llvm::Instruction *Inst);

  std::string getAnalysisName() { return "Overflow"; }
};

} // namespace dataflow

#endif // OVERFLOW_ANALYSIS_H
