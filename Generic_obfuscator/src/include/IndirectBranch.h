#ifndef LLVM_KObfucator_IndirectBranch_H
#define LLVM_KObfucator_IndirectBranch_H
// 现在仅适配了AArch64架构 因为不同架构之间汇编不一样
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include <map>
#include <vector>
#include <cstdio>
using namespace llvm;
namespace llvm {
class IndirectBranch : public PassInfoMixin<IndirectBranch> {
public:
    PreservedAnalyses run(Module& M, ModuleAnalysisManager& AM);
    // 保证不被跳过
    static bool isRequired() { return true; }
};
}
namespace KObfucator {
#include <llvm/IR/BasicBlock.h> // 包含 BasicBlock 所需的头文件

struct IndirectBBinfo {
  int index;
  int key;
};

}
#endif