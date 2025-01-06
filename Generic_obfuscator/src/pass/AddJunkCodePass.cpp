
#include "../include/AddJunkCodePass.h"
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

#include <cstdlib> // rand()
using namespace llvm;
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
    readConfig("/home/zzzccc/cxzz/Kotoamatsukami/config/config.json");
    if (Junkcode.model)
    {
        for (llvm::Function &F : M)
        {
            if (!F.hasExactDefinition())
            {
                continue;
            }
            int flowerCount = 0;
            int bb_index = 0;
            if (Junkcode.model == 2)
            {
                if (std::find(Junkcode.enable_function.begin(), Junkcode.enable_function.end(), F.getName()) == Junkcode.enable_function.end())
                {
                    continue;
                }
            }
            else if (Junkcode.model == 3)
            {
                if (std::find(Junkcode.disable_function.begin(), Junkcode.disable_function.end(), F.getName()) != Junkcode.disable_function.end())
                {
                    continue;
                }
            }
            llvm::outs() << "Running JunkcodePass on function: " << F.getName() << "\n";
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
                        int flowerClass = rand() % 2;
                        // int flowerClass = 1;
                        std::string asmCode;
                        if (flowerClass == 0)
                        {
                            if (targetArch == Arch::X86)
                            {
                                asmCode = llvm::formatv(
                                    x86AsmCode[flowerClass].c_str(),
                                    flowerIndex++);
                            }
                            else if (targetArch == Arch::X86_64)
                            {
                                asmCode = llvm::formatv(
                                    x64AsmCode[flowerClass].c_str(),
                                    flowerIndex++);
                            }
                        }
                        else
                        {
                            if (targetArch == Arch::X86)
                            {
                                asmCode = llvm::formatv(
                                    x86AsmCode[flowerClass].c_str(),
                                    flowerIndex++,
                                    flowerIndex++);
                            }
                            else if (targetArch == Arch::X86_64)
                            {
                                asmCode = llvm::formatv(
                                    x64AsmCode[flowerClass].c_str(),
                                    flowerIndex++,
                                    flowerIndex++);
                            }
                        }
                        // llvm::outs() << archToString(targetArch) <<"\n";
                        InlineAsm *rawAsm = llvm::InlineAsm::get(FType, asmCode, "",
                                                                 /* hasSideEffects */ true,
                                                                 /* isStackAligned */ true);
                        builder.CreateCall(rawAsm);
                        flowerCount++;
                        isChanged = true;
                    }
                }
            }
        }
    }

    if (isChanged)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}