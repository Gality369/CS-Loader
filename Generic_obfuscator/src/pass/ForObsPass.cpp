// 改进 将驶出条件改为 二元一次方程或其他较复杂的
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "../include/ForObsPass.h"
#include "config.h"
#include <cstdlib> // 用于随机数生成
#include <set>     // 用于跟踪已处理的基本块

using namespace llvm;
std::set<BasicBlock *> forpass_processedBlocks; // 用于跟踪已处理的基本块
std::set<BasicBlock *> generatedBlocks;         // 用于跟踪已生成循环的基本块

// 在指定的基本块中插入二重循环的函数
void insertNestedLoop(IRBuilder<> &builder, LLVMContext &context, Function &F,
                      Value *N, Value *M, IntegerType *intType, BasicBlock *OldBB, BasicBlock *NewBB,BasicBlock *NewNewBB,  int k)
{
    BasicBlock *TmpLoopCond = nullptr;
    BasicBlock *TmpLoopBody = nullptr;
    BasicBlock *TmpLoopEnd = nullptr;

    // 创建循环初始值
    Value *outerLoopVar = builder.CreateAlloca(intType, nullptr, "i");
    builder.CreateStore(ConstantInt::get(intType, 0), outerLoopVar);
    std::vector<Value *> innerLoopVarVec;
    for (int i = 0; i < k; i++)
    {
        std::string tmpName = "innerLoopVar_" + std::to_string(i);
        Value *tmpVar = builder.CreateAlloca(intType, nullptr, tmpName);
        builder.CreateStore(ConstantInt::get(intType, 0), tmpVar);
        innerLoopVarVec.push_back(tmpVar);
    }
    Value *isExcutedVal = builder.CreateAlloca(intType, nullptr, "is_excuted");
    builder.CreateStore(ConstantInt::get(intType, 0), outerLoopVar);

    // 创建中介基本块 关联循环与原基本块
    BasicBlock *excuteCond = BasicBlock::Create(context, "excute.cond", &F);
    builder.SetInsertPoint(excuteCond);
    generatedBlocks.insert(excuteCond);
    Value *isExcuteLoad = builder.CreateLoad(intType, isExcutedVal);
    Value *isExcuteNext = builder.CreateAdd(isExcuteLoad, ConstantInt::get(intType, 1));
    builder.CreateStore(isExcuteNext, isExcutedVal);

    // builder.CreateCondBr(isExcuteCond, innerLoopBody, innerLoopEnd);

    // 创建外循环基本块
    BasicBlock *outerLoopCond = BasicBlock::Create(context, "outer.loop.cond", &F);
    generatedBlocks.insert(outerLoopCond);
    BasicBlock *outerLoopBody = BasicBlock::Create(context, "outer.loop.body", &F);
    generatedBlocks.insert(outerLoopBody);
    BasicBlock *outerLoopEnd = BasicBlock::Create(context, "outer.loop.end", &F);
    generatedBlocks.insert(outerLoopEnd);
    Instruction *newBr = BranchInst::Create(outerLoopCond);

    // 使用 ReplaceInstWithInst 将旧的终止指令替换为新的跳转指令
    ReplaceInstWithInst(OldBB->getTerminator(), newBr);
    // F.getParent()->print(llvm::outs(), nullptr);
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

    // F.getParent()->print(llvm::outs(), nullptr);
    TmpLoopCond = outerLoopCond;
    TmpLoopBody = outerLoopBody;
    TmpLoopEnd = outerLoopEnd;
    // F.print(llvm::errs(), nullptr);

    for (int i = 0; i < k; i++)
    {
        // 插入外循环体
        builder.SetInsertPoint(TmpLoopBody);

        // 创建内循环初始值
        Value *innerLoopVar = innerLoopVarVec[i];
        
        // 创建内循环基本块
        BasicBlock *innerLoopCond = BasicBlock::Create(context, "inner.loop.cond", &F);
        generatedBlocks.insert(innerLoopCond);
        BasicBlock *innerLoopBody = BasicBlock::Create(context, "inner.loop.body", &F);
        generatedBlocks.insert(innerLoopBody);
        BasicBlock *innerLoopEnd = BasicBlock::Create(context, "inner.loop.end", &F);
        generatedBlocks.insert(innerLoopEnd);

        builder.CreateBr(innerLoopCond); // 跳转到内循环条件检查块
        builder.SetInsertPoint(innerLoopCond);
        
        // 内循环条件判断
        Value *innerLoad = builder.CreateLoad(intType, innerLoopVar);
        Value *innerCond = builder.CreateICmpSLT(innerLoad, M);
        builder.CreateCondBr(innerCond, innerLoopBody, innerLoopEnd);
        
        // 插入内循环体
        builder.SetInsertPoint(innerLoopBody);
        // 这里可以插入内循环体的指令

        // 内循环结束，递增内循环变量
        Value *innerNext = builder.CreateAdd(innerLoad, ConstantInt::get(intType, 1));
        builder.CreateStore(innerNext, innerLoopVar);
        if (i == (k - 1))
        {
            Value *isExcuteLoad = builder.CreateLoad(intType, isExcutedVal);
            Value *isExcuteCond = builder.CreateICmpSGT(isExcuteLoad, ConstantInt::get(intType, 0));
            builder.CreateCondBr(isExcuteCond, innerLoopCond, excuteCond);
            builder.SetInsertPoint(excuteCond);
            builder.CreateBr(NewBB);

        }

        // 内循环结束，跳回外循环
        builder.SetInsertPoint(innerLoopEnd);
        builder.CreateBr(TmpLoopCond); // 跳回外循环体块
        

        TmpLoopCond = innerLoopCond;
        TmpLoopBody = innerLoopBody;
        TmpLoopEnd = innerLoopEnd;
        
    }
}

PreservedAnalyses ForObsPass::run(Module &M, ModuleAnalysisManager &AM)
{
    bool IsChanged = false;
    double Probability = 0.5;
    // 初始化随机种子
    srand(time(nullptr));
    // 获取 LLVM 上下文和变量类型
    LLVMContext &context = M.getContext();
    IntegerType *intType = Type::getInt32Ty(context);
    Value *N = ConstantInt::get(intType, 10); // 外循环边界
    Value *M1 = ConstantInt::get(intType, 5);  // 内循环边界
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    if (ForObs.model){
        for (llvm::Function &F : M) {
            int addCount = 0;
            if(ForObs.model == 2){
                if(std::find(ForObs.enable_function.begin(),ForObs.enable_function.end(),F.getName()) == ForObs.enable_function.end()){
                    continue;                    
                }
            }else if (ForObs.model == 3)
            {                
                if(std::find(ForObs.disable_function.begin(),ForObs.disable_function.end(),F.getName()) != ForObs.disable_function.end()){
                    continue;                    
                }
            }
            
            llvm::outs() << "Running ForObsPass on function: " << F.getName() << "\n";

            // 遍历函数中的每个基本块
            for (BasicBlock &BB : F)
            {
                IRBuilder<> Builder(context);
                if (BB.size() < 2)
                {
                    // llvm::errs() << "Skipping block with less than 4 instructions: " << BB.getName() << "\n";
                    continue;
                }
                // 确保基本块没有被处理或生成过循环
                if (forpass_processedBlocks.count(&BB) == 0 && generatedBlocks.count(&BB) == 0)
                {
                    // 第一步：选择一个随机的指令作为拆分点
                    Instruction *splitInst = nullptr;
                    unsigned instructionIndex = rand() % (BB.size() - 1);
                    auto it = BB.begin();
                    std::advance(it, instructionIndex);
                    splitInst = &(*it);
                    // 第二步：在选定的指令处拆分基本块
                    BasicBlock *newBB = SplitBlock(&BB, splitInst);
                    
                    // 再拆一次
                    instructionIndex = rand() % (newBB->size() - 1);
                    auto it2 = newBB->begin();
                    std::advance(it2, instructionIndex);
                    splitInst = &(*it2);  
                    BasicBlock *newnewBB = SplitBlock(newBB, splitInst);

                    // 第三步：为每层循环分配内存给循环变量，并初始化它
                    Builder.SetInsertPoint(&*BB.getFirstInsertionPt());
                    // 生成一个 0 到 1 之间的随机浮点数，并与概率进行比较
                    if ((rand() / (double)RAND_MAX) < Probability )
                    {
                        if(addCount >= 3){
                            break;
                        }    
                        // 调用 insertNestedLoop 函数插入二重循环
                        insertNestedLoop(Builder, context, F, N, M1, intType, &BB, newBB,newnewBB, 4);

                        // 标记基本块已修改
                        IsChanged = true;

                        // 将当前基本块添加到已处理和已生成循环的集合中
                        forpass_processedBlocks.insert(&BB);
                        generatedBlocks.insert(newBB); // 标记新生成的基本块
                        generatedBlocks.insert(newnewBB); 
                        llvm::outs() << "Inserted nested loop into Func: " << F.getName() << "\n";
                        addCount++;
                    }
                }
            }

        }}

    // 返回适当的分析结果
    if (IsChanged)
    {
        return PreservedAnalyses::none();
    }
    else
    {
        return PreservedAnalyses::all();
    }
}
