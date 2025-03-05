#ifndef LLVM_Generic_obfuscator_Loopen_H
#define LLVM_Generic_obfuscator_Loopen_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include <map>
#include <set>
#include <cstdlib> // 包含 srand 和 rand
#include <ctime>   // 包含 time
#include <random>
#include <string>
#include <algorithm>   // std::shuffle
namespace llvm
{
    class Loopen : public PassInfoMixin<Loopen>
    {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
        // 保证不被跳过
        static bool isRequired() { return true; }
        
    };

} // namespace llvm
llvm::Function *createQuickPow(llvm::Module *M, std::string &moduleName);
void funcLoopen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Function &F, llvm::Function *quickPowFunc, llvm::Value *N, llvm::Value *M, llvm::BasicBlock *OldBB);
unsigned int quick_pow(unsigned int base, unsigned int exp, unsigned int mod);

#endif