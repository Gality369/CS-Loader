#include "BogusControlFlow.h"
#include "config.h"
#include "utils.hpp"
#include "llvm-c/Types.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <istream>
using namespace llvm;
namespace KObfucator {
// 魔改思路 将每个被魔改的都改成一个类似while循环 当flag为0 时才能跳出循环 然后每次都让 x + 1 不用 y 了
namespace BogusControlFlow {
    // 含有相关指令的clone block 不能被运行 这里可能不一定完备
    bool containsSpecialInstructions(llvm::BasicBlock& BB)
    {
        for (llvm::Instruction& I : BB) {
            if (llvm::isa<llvm::CallInst>(I) || llvm::isa<llvm::InvokeInst>(I)) {
                return true;
            }
        }
        return false;
    }
    Value* createMayRunBogusCmp(BasicBlock* insertAfter)
    {
        // if(( x * (x + 1) % 72 == 0))
        Module* M = insertAfter->getModule();
        LLVMContext& context = M->getContext();
        GlobalVariable* xptr = (GlobalVariable*)M->getOrInsertGlobal("x", Type::getInt32Ty(context));
        if (!xptr->hasInitializer()) {
            xptr->setInitializer(ConstantInt::get(Type::getInt32Ty(context),2)); // 初始值为 0
        }
        xptr->setLinkage(GlobalValue::CommonLinkage);
        IRBuilder<> builder(context);
        builder.SetInsertPoint(insertAfter);
        LoadInst* x = builder.CreateLoad(Type::getInt32Ty(context), xptr);
        Value* op1 = builder.CreateAdd(x, ConstantInt::get(Type::getInt32Ty(context), 1));
        Value* op2 = builder.CreateMul(x, op1);
        builder.CreateStore(op1, xptr);
        int randNum = getRandomNumber()%5201314;
        Value* cond = builder.CreateICmpEQ(op2, ConstantInt::get(Type::getInt32Ty(context), randNum));
        return cond;
    }
    Value* createBogusCmp(BasicBlock* insertAfter)
    {
        // if(( x * (x + 1) % 72 == 0))
        Module* M = insertAfter->getModule();
        LLVMContext& context = M->getContext();
        GlobalVariable* xptr = (GlobalVariable*)M->getOrInsertGlobal("x", Type::getInt32Ty(context));
        if (!xptr->hasInitializer()) {
            xptr->setInitializer(ConstantInt::get(Type::getInt32Ty(context),2)); // 初始值为 0
        }

        IRBuilder<> builder(context);
        builder.SetInsertPoint(insertAfter);
        LoadInst* x = builder.CreateLoad(Type::getInt32Ty(context), xptr);
        Value* op1 = builder.CreateAdd(x, ConstantInt::get(Type::getInt32Ty(context), 1));
        Value* op2 = builder.CreateMul(op1, x);
        Value* op3 = builder.CreateURem(op2, ConstantInt::get(Type::getInt32Ty(context), 2));
        builder.CreateStore(op3, xptr);
        Value* cond = builder.CreateICmpEQ(op3, ConstantInt::get(Type::getInt32Ty(context), 0));
        return cond;
    }
    BasicBlock* createJump2BodyBB(Function* F, Value* flag_ptr, BasicBlock* bodyBB, BasicBlock* tailBB)
    {
        Module* M = F->getParent();
        LLVMContext& context = M->getContext();
        IRBuilder<> builder(context);
        llvm::BasicBlock* jump2BodyBB = llvm::BasicBlock::Create(context, "jump2BodyBB", F);
        builder.SetInsertPoint(jump2BodyBB);
        Value* flagCond = builder.CreateLoad(Type::getInt1Ty(context), flag_ptr);
        builder.CreateCondBr(flagCond, bodyBB, tailBB);
        return jump2BodyBB;
    }
    llvm::Instruction* getFirstAllocaOrLastInstruction(llvm::BasicBlock& BB)
    {
        for (auto& inst : BB) {
            if (llvm::isa<llvm::AllocaInst>(&inst)) {
                return &inst;
            }
        }
        if (!BB.empty()) {
            return &BB.back();
        }

        return nullptr;
    }
}
} // namespace KObfucator
PreservedAnalyses BogusControlFlow::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    bool is_processed = false;
    if (bogus_control_flow.model) {
        for (llvm::Function& F : M) {
            if (bogus_control_flow.model == 2) {
                if (std::find(bogus_control_flow.enable_function.begin(), bogus_control_flow.enable_function.end(), F.getName()) == bogus_control_flow.enable_function.end()) {
                    continue;
                }
            } else if (bogus_control_flow.model == 3) {
                if (std::find(bogus_control_flow.disable_function.begin(), bogus_control_flow.disable_function.end(), F.getName()) != bogus_control_flow.disable_function.end()) {
                    continue;
                }
            }
            if (!F.hasExactDefinition()) {
                continue;
            }
            // 申请一个局部变量 用来保证real block 确实会被运行
            BasicBlock& entryBB = F.getEntryBlock();
            IRBuilder<> builder(F.getContext());
            builder.SetInsertPoint(KObfucator::BogusControlFlow::getFirstAllocaOrLastInstruction(entryBB));
            Value* flag_ptr = builder.CreateAlloca(Type::getInt1Ty(F.getContext()));
            builder.CreateStore(ConstantInt::get(Type::getInt1Ty(F.getContext()), 0), flag_ptr);

            std::vector<BasicBlock*> origBB;
            for (BasicBlock& BB : F) {
                origBB.push_back(&BB);
            }
            for (BasicBlock* BB : origBB) {
                // 控制插入概率
                if (isa<InvokeInst>(BB->getTerminator()) || BB->isEHPad() || (getRandomNumber() % 100) <= USE_PROBILITY) {
                    continue;
                }
                BasicBlock* headBB = BB;
                // 这个API好用啊
                BasicBlock* bodyBB = BB->splitBasicBlock(BB->getFirstNonPHIOrDbgOrLifetime(), "bodyBB");
                BasicBlock* tailBB = bodyBB->splitBasicBlock(bodyBB->getTerminator(), "endBB");
                BasicBlock* cloneBB = cloneBasicBlock(bodyBB);

                BB->getTerminator()->eraseFromParent();
                bodyBB->getTerminator()->eraseFromParent();
                cloneBB->getTerminator()->eraseFromParent();

                if ((getRandomNumber() % 100) <= 50) {
                    Value* cond1 = KObfucator::BogusControlFlow::createBogusCmp(headBB);
                    BranchInst::Create(bodyBB, cloneBB, cond1, headBB);
                    BranchInst::Create(tailBB, cloneBB, cond1, bodyBB);
                    BranchInst::Create(bodyBB, cloneBB);
                } else {
                    Value* cond1 = KObfucator::BogusControlFlow::createMayRunBogusCmp(headBB);
                    BasicBlock* jump2BodyBB = KObfucator::BogusControlFlow::createJump2BodyBB(&F, flag_ptr, bodyBB, tailBB);
                    BranchInst::Create(cloneBB, bodyBB, cond1, headBB);
                    BranchInst::Create(jump2BodyBB, bodyBB);
                    BranchInst::Create(jump2BodyBB, cloneBB);
                    builder.SetInsertPoint(bodyBB->getTerminator());
                    builder.CreateStore(ConstantInt::get(Type::getInt1Ty(F.getContext()), 0), flag_ptr);
                    builder.SetInsertPoint(cloneBB->getTerminator());
                    builder.CreateStore(ConstantInt::get(Type::getInt1Ty(F.getContext()), 1), flag_ptr);
                }
            }
            demoteRegisters(&F);
            is_processed = true;
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}