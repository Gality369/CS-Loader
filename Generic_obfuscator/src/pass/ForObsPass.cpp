// 改进 将驶出条件改为 二元一次方程或其他较复杂的
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "../include/ForObsPass.h"
#include "Log.hpp"
#include "config.h"
#include <cstdlib> // 用于随机数生成
#include <set>     // 用于跟踪已处理的基本块
#include <random>

using namespace llvm;
std::set<BasicBlock *> forobsProcessedBlocks; // 用于跟踪已处理的基本块
std::set<BasicBlock *> generatedBlocks;         // 用于跟踪已生成循环的基本块

// Insert a multiple loop between 2 basic block
void insertMultipleLoop(IRBuilder<> &builder, LLVMContext &context, Function &F,
                        Value *N, Value *M, BasicBlock *OldBB, BasicBlock *NewBB, int k)
{
    // 生成一个随机后缀，避免名称冲突
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 99999999);
    int randomSuffix = dis(gen);  // 生成一个 1000-9999 的随机数

    BasicBlock *tmpLoopCond = nullptr;
    BasicBlock *tmpLoopBody = nullptr;
    BasicBlock *tmpLoopEnd = nullptr;
    IntegerType *intType = Type::getInt32Ty(context);

    // 添加随机后缀到变量名
    std::string outerLoopVarName = "i_" + std::to_string(randomSuffix);
    Value *outerLoopVar = builder.CreateAlloca(intType, nullptr, outerLoopVarName);
    builder.CreateStore(ConstantInt::get(intType, 0), outerLoopVar);

    std::vector<Value *> innerLoopVarVec;
    for (int i = 0; i < k; i++)
    {
        std::string tmpName = "innerLoopVar_" + std::to_string(i) + "_" + std::to_string(randomSuffix);
        Value *tmpVar = builder.CreateAlloca(intType, nullptr, tmpName);
        builder.CreateStore(ConstantInt::get(intType, 0), tmpVar);
        innerLoopVarVec.push_back(tmpVar);
    }

    std::string loopExitCondValName = "loopExitCondVal_" + std::to_string(randomSuffix);
    Value *loopExitCondVal = builder.CreateAlloca(intType, nullptr, loopExitCondValName);
    builder.CreateStore(ConstantInt::get(intType, 0), loopExitCondVal);  // 修复：初始化 loopExitCondVal 而非未定义的 isExcutedVal
    builder.CreateStore(ConstantInt::get(intType, 0), outerLoopVar);

    // 创建中介基本块，添加随机后缀
    std::string loopExitCondName = "executeCond_" + std::to_string(randomSuffix);
    BasicBlock *loopExitCond = BasicBlock::Create(context, loopExitCondName, &F);
    builder.SetInsertPoint(loopExitCond);
    generatedBlocks.insert(loopExitCond);
    Value *loopExitCondValLoad = builder.CreateLoad(intType, loopExitCondVal);
    Value *loopExitCondValNext = builder.CreateAdd(loopExitCondValLoad, ConstantInt::get(intType, 1));
    builder.CreateStore(loopExitCondValNext, loopExitCondVal);

    // 创建外循环基本块，添加随机后缀
    std::string outerLoopCondName = "outerLoopCond_" + std::to_string(randomSuffix);
    BasicBlock *outerLoopCond = BasicBlock::Create(context, outerLoopCondName, &F);
    generatedBlocks.insert(outerLoopCond);
    std::string outerLoopBodyName = "outerLoopBody_" + std::to_string(randomSuffix);
    BasicBlock *outerLoopBody = BasicBlock::Create(context, outerLoopBodyName, &F);
    generatedBlocks.insert(outerLoopBody);
    std::string outerLoopEndName = "outerLoopEnd_" + std::to_string(randomSuffix);
    BasicBlock *outerLoopEnd = BasicBlock::Create(context, outerLoopEndName, &F);
    generatedBlocks.insert(outerLoopEnd);
    Instruction *newBr = BranchInst::Create(outerLoopCond);
    ReplaceInstWithInst(OldBB->getTerminator(), newBr);
    builder.SetInsertPoint(outerLoopCond);

    // 外循环条件判断
    Value *outerLoad = builder.CreateLoad(intType, outerLoopVar);
    Value *outerCond = builder.CreateICmpSLT(outerLoad, N);
    builder.CreateCondBr(outerCond, outerLoopBody, outerLoopEnd);

    builder.SetInsertPoint(outerLoopBody);
    Value *outerNext = builder.CreateAdd(outerLoad, ConstantInt::get(intType, 1));
    builder.CreateStore(outerNext, outerLoopVar);
    builder.SetInsertPoint(outerLoopEnd);
    builder.CreateBr(NewBB);

    tmpLoopCond = outerLoopCond;
    tmpLoopBody = outerLoopBody;
    tmpLoopEnd = outerLoopEnd;

    for (int i = 0; i < k; i++)
    {
        builder.SetInsertPoint(tmpLoopBody);
        Value *innerLoopVar = innerLoopVarVec[i];

        // 创建内循环基本块，添加随机后缀
        std::string innerLoopCondName = "innerLoopCond_" + std::to_string(i) + "_" + std::to_string(randomSuffix);
        BasicBlock *innerLoopCond = BasicBlock::Create(context, innerLoopCondName, &F);
        generatedBlocks.insert(innerLoopCond);
        std::string innerLoopBodyName = "innerLoopBody_" + std::to_string(i) + "_" + std::to_string(randomSuffix);
        BasicBlock *innerLoopBody = BasicBlock::Create(context, innerLoopBodyName, &F);
        generatedBlocks.insert(innerLoopBody);
        std::string innerLoopEndName = "innerLoopEnd_" + std::to_string(i) + "_" + std::to_string(randomSuffix);
        BasicBlock *innerLoopEnd = BasicBlock::Create(context, innerLoopEndName, &F);
        generatedBlocks.insert(innerLoopEnd);

        builder.CreateBr(innerLoopCond);
        builder.SetInsertPoint(innerLoopCond);

        Value *innerLoad = builder.CreateLoad(intType, innerLoopVar);
        Value *innerCond = builder.CreateICmpSLT(innerLoad, M);
        builder.CreateCondBr(innerCond, innerLoopBody, innerLoopEnd);

        builder.SetInsertPoint(innerLoopBody);
        Value *innerNext = builder.CreateAdd(innerLoad, ConstantInt::get(intType, 1));
        builder.CreateStore(innerNext, innerLoopVar);

        if (i == (k - 1))
        {
            Value *tmpLoad = builder.CreateLoad(intType, loopExitCondVal);
            Value *tmpCond = builder.CreateICmpSGT(tmpLoad, ConstantInt::get(intType, 0));
            // 修复：调整条件分支参数顺序
            builder.CreateCondBr(tmpCond, innerLoopCond, loopExitCond);
            builder.SetInsertPoint(loopExitCond);
            builder.CreateBr(NewBB);
        }

        builder.SetInsertPoint(innerLoopEnd);
        builder.CreateBr(tmpLoopCond);

        tmpLoopCond = innerLoopCond;
        tmpLoopBody = innerLoopBody;
        tmpLoopEnd = innerLoopEnd;
    }
}

PreservedAnalyses ForObsPass::run(Module &M, ModuleAnalysisManager &AM)
{
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    bool IsChanged = false;
    double Probability = 0.5;
    srand(time(nullptr));
    LLVMContext &context = M.getContext();
    IntegerType *intType = Type::getInt32Ty(context);
    Value *innerLoopBoundary; // 外循环边界
    Value *outerLoopBoundary;  // 内循环边界
    if (forObs.op1){
        innerLoopBoundary = ConstantInt::get(intType, forObs.op1);
    }else {
        innerLoopBoundary = ConstantInt::get(intType, 10);
    }
    if (forObs.op2){
        outerLoopBoundary = ConstantInt::get(intType, forObs.op2);
    }else {
        outerLoopBoundary = ConstantInt::get(intType, 5);
    }
    if (forObs.model){
        for (llvm::Function &F : M) {
            int addCount = 0;
            if (shouldSkip(F, forObs)) {
                continue;
            }
            PrintInfo("Running ForObsPass on function: " , F.getName().str());
            for (BasicBlock &BB : F)
            {
                IRBuilder<> Builder(context);
                if (BB.size() < 2)
                {
                    continue;
                }
                // Ensure that basic blocks have not been processed or generated loops
                if (forobsProcessedBlocks.count(&BB) == 0 && generatedBlocks.count(&BB) == 0)
                {
                    Instruction *firstNonSpecial = BB.getFirstNonPHIOrDbgOrLifetime();
                    if (!firstNonSpecial || firstNonSpecial == &BB.back()) {
                        continue;
                    }

                    auto beginIt = BB.begin();
                    auto endIt = BB.end();
                    --endIt;

                    unsigned validStartIndex = 0;
                    unsigned validEndIndex = BB.size() - 1;
                    for (auto it = BB.begin(); it != BB.end(); ++it, ++validStartIndex) {
                        if (&(*it) == firstNonSpecial) {
                            break;
                        }
                    }
                    if (validStartIndex >= validEndIndex) {
                        continue;
                    }

                    // choose the insert pos randomly
                    unsigned instructionIndex = validStartIndex + (rand() % (validEndIndex - validStartIndex));
                    auto it = BB.begin();
                    std::advance(it, instructionIndex);
                    Instruction *splitInst = &(*it);

                    BasicBlock *newBB = SplitBlock(&BB, splitInst);

                    Builder.SetInsertPoint(&*BB.getFirstInsertionPt());
                    if ((rand() / (double)RAND_MAX) < Probability)
                    {
                        // Prevent excessive insertion from affecting efficiency
                        if (addCount >= 3) {
                            break;
                        }
                        insertMultipleLoop(Builder, context, F, innerLoopBoundary, outerLoopBoundary, &BB,newBB, 4);
                        forobsProcessedBlocks.insert(&BB);
                        generatedBlocks.insert(newBB);
                        llvm::outs() << "Inserted nested loop into Func: " << F.getName() << "\n";
                        addCount++;
                        IsChanged = true;
                    }
                }
            }

        }}
    if (IsChanged)
    {
        return PreservedAnalyses::none();
    }
    else
    {
        return PreservedAnalyses::all();
    }
}
