#ifndef LLVM_Generic_obfuscator_IndirectCall_H
#define LLVM_Generic_obfuscator_IndirectCall_H
//现在仅适配了AArch64架构 因为不同架构之间汇编不一样
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include <map>
using namespace llvm;
namespace llvm
{
  class IndirectCall : public PassInfoMixin<IndirectCall>
  {
  public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    // 保证不被跳过
    static bool isRequired() { return true; }
  };
}
namespace Generic_obfuscator
{
  struct IndirectCallInfo {
  int index;
  int key;
};
} // namespace Generic_obfuscator

#endif