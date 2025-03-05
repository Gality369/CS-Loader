#include "../include/AddJunkCodePass.h"
#include "Log.hpp"
#include "config.h"
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
#include <string>
#include <vector>

#include <cstdlib> // rand()
using namespace llvm;

// todo https://zhuanlan.zhihu.com/p/640711859
std::vector<std::string> arm64AsmCode = {
    "udf #0x5397FB1\n"
    ".long 87654321\n"
    ".long 12345678\n",

    "b #0x10\n"
    "udf #0x5397FB1\n"
    ".long 87654321\n"
    ".long 12345678\n"
};
std::vector<std::string> x86AsmCode = {
    "call {0}f\n"
    "{0}:\n"
    "addl $$8, (%esp)\n"
    "ret\n"
    ".byte 0xe8\n"
    ".byte 0x90\n"
    ".byte 0x90\n"
    ".byte 0x90\n",

    "push %ebx\n"
    "xorl %ebx, %ebx\n"
    "testl %ebx, %ebx\n"
    "jne {0}f\n"
    "je {1}f\n"
    "{0}:\n"
    ".byte 0x21\n"
    "{1}:\n"
    "pop %ebx\n"};

std::vector<std::string> x64AsmCode = {
    "call {0}f\n"
    "{0}:\n"
    "addq $$8, (%rsp)\n"
    "ret\n"
    ".byte 0xe8\n"
    ".byte 0x90\n"
    ".byte 0x90\n"
    ".byte 0x90\n",

    "push %rbx\n"
    "xorq %rbx, %rbx\n"
    "testq %rbx, %rbx\n"
    "jne {0}f\n"
    "je {1}f\n"
    "{0}:\n"
    ".byte 0x21\n"
    "{1}:\n"
    "pop %rbx\n"};
PreservedAnalyses AddJunkCodePass::run(Module &M, ModuleAnalysisManager &AM)
{
    bool isChanged = false;
    int flowerIndex = 0;
    double addJunkCodeProbability = 0.2;
    srand(time(nullptr));
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    if (junkCode.model)
    {
        for (llvm::Function &F : M)
        {
            if (shouldSkip(F, junkCode)){
                continue;
            }
            int flowerCount = 0;
            PrintInfo("Running JunkcodePass on function: ",F.getName().str());
            for (auto &BB : F)
            {
                if (flowerCount >= 5)
                {
                    break;
                }
                for (auto &I : BB)
                {
                    if (I.isTerminator())
                    {
                        continue;
                    }
                    if ((rand() / (double)RAND_MAX) < addJunkCodeProbability)
                    {
                        IRBuilder<> builder(&I);
                        auto *FType = llvm::FunctionType::get(builder.getVoidTy(), false);
                        std::string asmCode;
                        if (targetArch == Arch::X86)
                        {
                            int flowerClass = rand() % x86AsmCode.size();
                            asmCode = llvm::formatv(
                                x86AsmCode[flowerClass].c_str(),
                                flowerIndex++);
                        }
                        else if (targetArch == Arch::X86_64)
                        {
                            int flowerClass = rand() % x64AsmCode.size();
                            asmCode = llvm::formatv(
                                x64AsmCode[flowerClass].c_str(),
                                flowerIndex++);
                        }
                        else if (targetArch == Arch::ARM64) {
                            int flowerClass = rand() % arm64AsmCode.size();
                            asmCode = llvm::formatv(
                                arm64AsmCode[flowerClass].c_str(),
                                flowerIndex++);
                        }
                        InlineAsm *rawAsm = llvm::InlineAsm::get(FType, asmCode, "",
                                                                 /* hasSideEffects */ true,
                                                                 /* isStackAligned */ true);
                        builder.CreateCall(rawAsm);
                        flowerCount++;
                        isChanged = true;
                    }
                }
            }
            PrintSuccess("AddJunkCodePass successfully process func ", F.getName().str());
        }
    }

    if (isChanged)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}