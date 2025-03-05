#include "utils.hpp"
#include "Loopen.hpp"
#include "config.h"
#include "Log.hpp"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/raw_ostream.h"
std::string quickPowIR = R"XYZ(
    ; ModuleID = 'quick_pow.c'
    source_filename = "quick_pow.c"
    target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
    target triple = "x86_64-pc-linux-gnu"
    
    ; Function Attrs: noinline nounwind optnone uwtable
    define dso_local i32 @Generic_obfuscator_quick_pow(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
      %4 = alloca i32, align 4
      %5 = alloca i32, align 4
      %6 = alloca i32, align 4
      %7 = alloca i32, align 4
      %8 = alloca i32, align 4
      store i32 %0, ptr %5, align 4
      store i32 %1, ptr %6, align 4
      store i32 %2, ptr %7, align 4
      store i32 1, ptr %8, align 4
      %9 = load i32, ptr %5, align 4
      %10 = load i32, ptr %7, align 4
      %11 = urem i32 %9, %10
      store i32 %11, ptr %5, align 4
      %12 = load i32, ptr %6, align 4
      %13 = icmp ne i32 %12, 2
      br i1 %13, label %14, label %15
    
    14:                                               ; preds = %3
      store i32 0, ptr %4, align 4
      br label %39
    
    15:                                               ; preds = %3
      br label %16
    
    16:                                               ; preds = %29, %15
      %17 = load i32, ptr %6, align 4
      %18 = icmp ugt i32 %17, 0
      br i1 %18, label %19, label %37
    
    19:                                               ; preds = %16
      %20 = load i32, ptr %6, align 4
      %21 = urem i32 %20, 2
      %22 = icmp eq i32 %21, 1
      br i1 %22, label %23, label %29
    
    23:                                               ; preds = %19
      %24 = load i32, ptr %8, align 4
      %25 = load i32, ptr %5, align 4
      %26 = mul i32 %24, %25
      %27 = load i32, ptr %7, align 4
      %28 = urem i32 %26, %27
      store i32 %28, ptr %8, align 4
      br label %29
    
    29:                                               ; preds = %23, %19
      %30 = load i32, ptr %5, align 4
      %31 = load i32, ptr %5, align 4
      %32 = mul i32 %30, %31
      %33 = load i32, ptr %7, align 4
      %34 = urem i32 %32, %33
      store i32 %34, ptr %5, align 4
      %35 = load i32, ptr %6, align 4
      %36 = udiv i32 %35, 2
      store i32 %36, ptr %6, align 4
      br label %16, !llvm.loop !6
    
    37:                                               ; preds = %16
      %38 = load i32, ptr %8, align 4
      store i32 %38, ptr %4, align 4
      br label %39
    
    39:                                               ; preds = %37, %14
      %40 = load i32, ptr %4, align 4
      ret i32 %40
    }
    
    attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
    
    !llvm.module.flags = !{!0, !1, !2, !3, !4}
    !llvm.ident = !{!5}
    
    !0 = !{i32 1, !"wchar_size", i32 4}
    !1 = !{i32 8, !"PIC Level", i32 2}
    !2 = !{i32 7, !"PIE Level", i32 2}
    !3 = !{i32 7, !"uwtable", i32 2}
    !4 = !{i32 7, !"frame-pointer", i32 2}
    !5 = !{!"Ubuntu clang version 17.0.6 (9ubuntu1)"}
    !6 = distinct !{!6, !7}
    !7 = !{!"llvm.loop.mustprogress"}
    )XYZ";
// #include "llvm/Transforms/Utils/LowerSwitch.h"
//  namespace
// The modulus needs to be large enough to ensure that the square of x does not repeat
#define QUICK_POW 2147483647
using namespace llvm;
std::vector<BasicBlock *> origBB;
std::map<BasicBlock *, int> BB2X;
std::map<BasicBlock *, int> BB2Y;
std::set<int> resultSet;

int xCount = 0;
// Values ​​other than 2 return 0 to ensure efficiency.
unsigned int quick_pow(unsigned int base, unsigned int exp, unsigned int mod)
{
    unsigned int result = 1;
    base = base % mod;
    if(exp != 2){
        return 0;
    }
    while (exp > 0)
    {
        if (exp % 2 == 1)
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp /= 2;
    }

    return result;
}

llvm::Function *createQuickPow(llvm::Module *M,std::string &moduleName)
{
    std::string funcName = "Generic_obfuscator_quick_pow";

    // 在模块中检查是否已经存在同名的函数
    llvm::Function *existingFunc = M->getFunction(funcName);
    if (existingFunc)
    {
        // 如果函数已存在，直接返回该函数
        return existingFunc;
    }
    // PrintInfo("Loopen start createFuncFromGenerated ");
    llvm::Function *newFunc = createFuncFromString(M, funcName, quickPowIR);
    newFunc->setLinkage(llvm::GlobalValue::InternalLinkage);
    // PrintInfo("[Loopen]: createFuncFromGenerated successfully ");
    // 返回新创建的函数
    return newFunc;
}

//  Loopen the specified function
void funcLoopen(IRBuilder<> &builder, LLVMContext &context, Function &F, Function *quickPowFunc, Value *xmax, Value *ymax, BasicBlock *EntryBB)
{
    IntegerType *intType = llvm::Type::getInt32Ty(context);

    // Create outer loop basic blocks
    BasicBlock *outerLoopCond = BasicBlock::Create(context, "outer.loop.cond", &F);
    BasicBlock *outerLoopBody = BasicBlock::Create(context, "outer.loop.body", &F);
    BasicBlock *outerLoopEnd = BasicBlock::Create(context, "outer.loop.end", &F);

    // Basic blocks for creating initial data
    BasicBlock *NewEntry = BasicBlock::Create(context, "new_entry", &F, EntryBB);
    builder.SetInsertPoint(NewEntry);
    // x
    Value *outerLoopVar = builder.CreateAlloca(intType, nullptr, "outerLoopVar");
    builder.CreateStore(ConstantInt::get(intType, 1), outerLoopVar);
    // y
    Value *innerLoopVar = builder.CreateAlloca(intType, nullptr, "innerLoopVar");
    builder.CreateStore(ConstantInt::get(intType, 0), innerLoopVar);

    // mod of x**y
    Value *quick_pow_mod = builder.CreateAlloca(intType, nullptr, "quick_pow_mod");
    builder.CreateStore(ConstantInt::get(intType, QUICK_POW), quick_pow_mod);

    //result of x**y
    Value *quick_pow_result = builder.CreateAlloca(intType, nullptr, "quick_pow_result");
    builder.CreateStore(ConstantInt::get(intType, 0), quick_pow_result);

    builder.CreateBr(outerLoopCond);
    // Move the original instructions to the new entry_block to prevent the scope of variables from being changed.
    for (auto it = EntryBB->begin(), end = std::prev(EntryBB->end()); it != end;)
    {
        Instruction &Inst = *it++;
        Inst.moveBefore(NewEntry->getTerminator()); 
    }
    
    BasicBlock *innerLoopCond = BasicBlock::Create(context, "inner.loop.cond", &F);
    BasicBlock *innerLoopBody = BasicBlock::Create(context, "inner.loop.body", &F);
    BasicBlock *innerLoopEnd = BasicBlock::Create(context, "inner.loop.end", &F);

    builder.SetInsertPoint(outerLoopCond);
    Value *outerLoad = builder.CreateLoad(intType, outerLoopVar);
    Value *outerCond = builder.CreateICmpULT(outerLoad, xmax);
    builder.CreateCondBr(outerCond, outerLoopBody, outerLoopEnd);

    // outerLoopBody
    builder.SetInsertPoint(outerLoopBody);
    Value *outerNext = builder.CreateAdd(outerLoad, ConstantInt::get(intType, 1));
    builder.CreateStore(outerNext, outerLoopVar);
    builder.CreateStore(ConstantInt::get(intType, 0), innerLoopVar);
    builder.CreateBr(innerLoopCond);

    // outerLoopCond
    builder.SetInsertPoint(outerLoopEnd);
    builder.CreateStore(ConstantInt::get(intType, 2), outerLoopVar);
    builder.CreateStore(ConstantInt::get(intType, 0), innerLoopVar);
    builder.CreateBr(outerLoopEnd);

    // innerLoopCond
    builder.SetInsertPoint(innerLoopCond);
    Value *innerLoad = builder.CreateLoad(intType, innerLoopVar);
    Value *innerCond = builder.CreateICmpULT(innerLoad, ymax);
    builder.CreateCondBr(innerCond, innerLoopBody, innerLoopEnd);

    // innerLoopBody
    builder.SetInsertPoint(innerLoopBody);
    Value *innerNext = builder.CreateAdd(innerLoad, ConstantInt::get(intType, 1));
    builder.CreateStore(innerNext, innerLoopVar);
    Value *outerLoopVarValue = builder.CreateLoad(intType, outerLoopVar); // x
    Value *innerLoopVarValue = builder.CreateLoad(intType, innerLoopVar); // y
    Value *quickPowModValue = builder.CreateLoad(intType, quick_pow_mod); // mod
    llvm::Value *callResult = builder.CreateCall(quickPowFunc, {outerLoopVarValue, innerLoopVarValue, quickPowModValue});

    // Insert alloca and store instructions in the entry block to create and initialize the switch variable, with the initial value being a random value.
    SwitchInst *swInst = SwitchInst::Create(callResult, innerLoopCond, 0, innerLoopBody);
    // Insert the original basic block before the return block and assign the case value
    for (BasicBlock *BB : origBB)
    {
        int randY = 2;        
        swInst->addCase(ConstantInt::get(intType, quick_pow(x[xCount], randY, QUICK_POW)), BB);
        resultSet.insert(quick_pow(x[xCount], randY, QUICK_POW));
        BB2X[BB] = x[xCount];
        BB2Y[BB] = randY;
        xCount += 1;
    }

    // At the end of each basic block, add instructions to modify the switch variable and jump to the return block.
    for (BasicBlock *BB : origBB)
    {
        // retn BB
        if (BB->getTerminator()->getNumSuccessors() == 0)
        {
            continue;
        }
        // Unconditional jump
        else if (BB->getTerminator()->getNumSuccessors() == 1)
        {
            BasicBlock *sucBB = BB->getTerminator()->getSuccessor(0);
            int _x = BB2X[sucBB] - 1;
            int _y = BB2Y[sucBB];
            builder.SetInsertPoint(&*BB->getFirstInsertionPt());
            builder.CreateStore(ConstantInt::get(intType, _x), outerLoopVar);
            builder.CreateStore(ConstantInt::get(intType, _y), innerLoopVar);
            BranchInst *BR2innerLoopEnd = BranchInst::Create(innerLoopEnd);
            ReplaceInstWithInst(BB->getTerminator(), BR2innerLoopEnd);
        }
        // Conditional jump
        else if (BB->getTerminator()->getNumSuccessors() == 2)
        {
            builder.SetInsertPoint(&*BB->getTerminator());
            BasicBlock *sucBBTrue = BB->getTerminator()->getSuccessor(0);
            BasicBlock *sucBBFalse = BB->getTerminator()->getSuccessor(1);
            int xTrue = BB2X[sucBBTrue] - 1;
            int yTrue = BB2Y[sucBBTrue];
            int xFalse = BB2X[sucBBFalse] - 1;
            int yFalse = BB2Y[sucBBFalse];
            auto *br = dyn_cast<BranchInst>(BB->getTerminator());
            if (!br || !br->isConditional()) { // 确保是条件分支
                PrintError("Not a condition br");
                BB->getTerminator()->print(llvm::outs());
            }
            // BranchInst *br = cast<BranchInst>(BB->getTerminator());
            Value *selX = builder.CreateSelect(br->getCondition(), ConstantInt::get(intType, xTrue), ConstantInt::get(intType, xFalse), "");
            Value *selY = builder.CreateSelect(br->getCondition(), ConstantInt::get(intType, yTrue), ConstantInt::get(intType, yFalse), "");
            builder.CreateStore(selX, outerLoopVar);
            builder.CreateStore(selY, innerLoopVar);
            BranchInst *BR2innerLoopEnd = BranchInst::Create(innerLoopEnd);
            ReplaceInstWithInst(BB->getTerminator(), BR2innerLoopEnd);
        }
    }
    // innerLoopEnd
    builder.SetInsertPoint(innerLoopEnd);
    builder.CreateBr(outerLoopCond);
}

PreservedAnalyses Loopen::run(Module &M, ModuleAnalysisManager &AM)
{
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    if (loopen.model){
        for (llvm::Function &F : M) {
            xCount = 0;
            origBB.clear();
            BB2X.clear();
            BB2Y.clear();
            resultSet.clear();
            int blockCount = 0;
            if (shouldSkip(F, loopen)){
                continue;
            }
            if (F.getName().contains("quick_pow"))
            {
                continue;
            }
            if (F.size() <= 2)
            {
                continue;
            }
            for (BasicBlock &BB : F)
            {
                blockCount++;
            }
            llvm::Module *M = F.getParent();
            llvm::LLVMContext &context = M->getContext();
            IntegerType *intType = llvm::Type::getInt32Ty(context);
            IRBuilder<> Builder(context);
            Value *xMax = ConstantInt::get(intType, 2147483647); // outer loop boundary
            Value *yMax = ConstantInt::get(intType, 4);     // inner loop boundary
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            for (BasicBlock &BB : F)
            {
                origBB.push_back(&BB);
            }
            // Remove the first basic block from the vector and process it separately to ensure variable scope
            origBB.erase(origBB.begin());
            BasicBlock &entryBB = F.getEntryBlock();
            if (BranchInst *br = dyn_cast<BranchInst>(entryBB.getTerminator()))
            {
                if (br->isConditional())
                {
                    BasicBlock *newBB = entryBB.splitBasicBlock(br, "newBB");
                    origBB.insert(origBB.begin(), newBB);
                }
            }
            std::random_device rd;
            std::default_random_engine e(rd());
            std::shuffle(origBB.begin() + 1, origBB.end(), e);
            llvm::Function *quickPowFunc = createQuickPow(M,loopen.module_name);
            // 检查基本块中是否至少有两条指令
            if (std::distance(entryBB.begin(), entryBB.end()) >= 2)
            {
                // 获取基本块中最后一条指令的迭代器
                llvm::BasicBlock::iterator I = entryBB.end();
                --I; // 现在 I 指向最后一条指令
                --I; // 再向前移动一条，指向倒数第二条指令
                Builder.SetInsertPoint(&*I);
            }
            else
            {
                // llvm::errs() << "Not enough instructions to insert before the second last one.\n";
                continue;
            }
            // PrintInfo("start funcLoopen: ",F.getName().str());
            funcLoopen(Builder, context, F, quickPowFunc, xMax, yMax, &entryBB);
            // PrintInfo("start fix stack: ",F.getName().str());
            demoteRegisters(&F);
            PrintSuccess("Loopen successfully process func ", F.getName().str());
        }
    }

    return PreservedAnalyses::none();
}