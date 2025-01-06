#ifndef LLVM_Kotoamatsukami_BogusControlFlow_H
#define LLVM_Kotoamatsukami_BogusControlFlow_H
//现在仅适配了AArch64架构 因为不同架构之间汇编不一样
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "utils.hpp"
#include <map>
#define USE_PROBILITY 0
using namespace llvm;
namespace llvm
{
  class BogusControlFlow : public PassInfoMixin<BogusControlFlow>
  {
  public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    // 保证不被跳过
    static bool isRequired() { return true; }
  };
}
#endif