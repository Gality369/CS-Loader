#ifndef LLVM_Kotoamatsukami_ForObsPass_H
#define LLVM_Kotoamatsukami_ForObsPass_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class ForObsPass : public PassInfoMixin<ForObsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  //保证不被跳过
  static bool isRequired() { return true; }

  
};
} // namespace llvm

#endif