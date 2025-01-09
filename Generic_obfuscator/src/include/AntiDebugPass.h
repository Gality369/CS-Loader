#ifndef LLVM_KObfucator_AntiDebugPass_H
#define LLVM_KObfucator_AntiDebugPass_H

#include "llvm/IR/PassManager.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/IR/InlineAsm.h"
#include <vector>
#include <string>

namespace llvm {
class AntiDebugPass : public PassInfoMixin<AntiDebugPass> {
public:
  PreservedAnalyses run(Module &F, ModuleAnalysisManager &AM);
  //保证不被跳过
  static bool isRequired() { return true; }
  
};
} // namespace llvm
#endif