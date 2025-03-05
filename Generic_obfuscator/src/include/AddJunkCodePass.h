#ifndef LLVM_Generic_obfuscator_AddJunkCodePass_H
#define LLVM_Generic_obfuscator_AddJunkCodePass_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class AddJunkCodePass : public PassInfoMixin<AddJunkCodePass> {
public:
  PreservedAnalyses run(Module &F, ModuleAnalysisManager &AM);
  //保证不被跳过
  static bool isRequired() { return true; }
  
};
} // namespace llvm

#endif