#include "../include/Branch2Call.h"
/* Branch 2 Call 讲条件跳转指令转为call */

#include "Log.hpp"
#include "config.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <random>
#include <set>
#include <sstream>
using namespace llvm;
#define MaxBlockNumber 4096
namespace {
int maxBlockNumber = 0;
int block_count = 0;
std::vector<BasicBlock*> BBTargets;
std::set<BasicBlock*> processedBlocks;
std::set<BasicBlock*> unreachableBlocks;
GlobalVariable* AllFunctions_IndirectBrTargets = nullptr;

ArrayType* OuterArrayTy = nullptr;
ArrayType* ATy = nullptr;

static int function_count = 0;
static std::map<Function*, unsigned> FunctionIndexMap;
GlobalVariable* rsp_tmp = nullptr;

std::random_device RandomDevice;
std::mt19937 RandomEngine(RandomDevice());

long long int ComputeRegisterValue(unsigned FunctionID, unsigned ID1,
    unsigned TrueID, unsigned FalseID)
{
    assert(FunctionID < (1 << 16) && "FunctionID exceeds 16 bits");
    assert(ID1 < (1 << 16) && "ID1 exceeds 16 bits");
    assert(TrueID < (1 << 16) && "TrueID exceeds 16 bits");
    assert(FalseID < (1 << 16) && "FalseID exceeds 16 bits");
    long long int X9Value = 0;
    X9Value |= ((long long int)FunctionID << 48); // Function ID
    X9Value |= ((long long int)ID1 << 32); // RESERVED
    X9Value |= ((long long int)TrueID << 16); // True Block ID
    X9Value |= (long long int)FalseID; // False Block ID
    return X9Value;
}

static std::map<BasicBlock*, unsigned> BBNumbering; // the mapping relationship between BasicBlock and the index
static int BBNumberingCount = 0;
std::vector<Constant*> ExistingFunctionArrays;
GlobalVariable* getIndirectTargetsMap(Module& M, Function* F,
    GlobalVariable*& GV)
{
    std::string GVName = "AllFunctions_IndirectBrTargets";
    if (!GV) {
        GV = M.getNamedGlobal(GVName);
    }
    if (ExistingFunctionArrays.size() == 0) {
        for (int k = 0; k < function_count; k++) {
            ExistingFunctionArrays.push_back(ConstantArray::get(
                ArrayType::get(Type::getInt8PtrTy(F->getContext()), maxBlockNumber),
                Constant::getNullValue(Type::getInt8PtrTy(F->getContext()))));
        }
    }
    std::vector<Constant*> Elements;
    for (auto& BB : *F) {
        if (&BB == &F->getEntryBlock()) {
            continue;
        }
        Constant* CE = ConstantExpr::getBitCast(
            BlockAddress::get(F, &BB), Type::getInt8PtrTy(F->getContext()));
        Elements.push_back(CE);
        BBNumbering[&BB] = BBNumberingCount++;
    }

    while (Elements.size() < maxBlockNumber) {
        Elements.push_back(
            Constant::getNullValue(Type::getInt8PtrTy(F->getContext())));
    }

    ATy = ArrayType::get(Type::getInt8PtrTy(F->getContext()), maxBlockNumber);
    Constant* CA = ConstantArray::get(ATy, ArrayRef<Constant*>(Elements));
    ExistingFunctionArrays[FunctionIndexMap[F]] = CA;
    OuterArrayTy = ArrayType::get(
        ArrayType::get(Type::getInt8PtrTy(F->getContext()), maxBlockNumber),
        function_count);
    while (ExistingFunctionArrays.size() < function_count) {
        ExistingFunctionArrays.push_back(ConstantArray::get(
            ArrayType::get(Type::getInt8PtrTy(F->getContext()), MaxBlockNumber),
            Constant::getNullValue(Type::getInt8PtrTy(F->getContext()))));
    }
    Constant* OuterArray = ConstantArray::get(
        OuterArrayTy, ArrayRef<Constant*>(ExistingFunctionArrays));
    if (!GV) {
        GV = new GlobalVariable(M, OuterArrayTy, false,
            GlobalValue::LinkageTypes::PrivateLinkage,
            OuterArray, GVName);
    } else {
        GV->eraseFromParent();
        GV = new GlobalVariable(M, OuterArrayTy, false,
            GlobalValue::LinkageTypes::PrivateLinkage,
            OuterArray, GVName);
    }
    appendToCompilerUsed(M, { GV });
    return GV;
}
// If the block have Only one predecessor
bool hasUniquePredecessor(llvm::BasicBlock* BB)
{
    int numPredecessors = std::distance(pred_begin(BB), pred_end(BB));
    return numPredecessors == 1;
}

int ProcessPredecessorsAndInsertFuncCall(Function& F, BasicBlock& BB,
    unsigned FunctionID, Function* Func1,
    Function* ConditionalJumpFunc_byX10)
{
    LLVMContext& Ctx = F.getContext();
    if (processedBlocks.count(&BB)) {
        return 0;
    }
    BranchInst* terminator = dyn_cast<BranchInst>(BB.getTerminator());
    if (llvm::BranchInst* BI = llvm::dyn_cast<llvm::BranchInst>(terminator)) {
        if (BI->isConditional()) {
            if (hasUniquePredecessor(BI->getSuccessor(0)) && hasUniquePredecessor(BI->getSuccessor(1))) {
                Value* Cond = BI->getCondition();
                IRBuilder<> Builder(F.getContext());
                Builder.SetInsertPoint(BI);
                long long int x9 = ComputeRegisterValue(
                    FunctionID, BBNumbering[&BB], BBNumbering[BI->getSuccessor(0)],
                    BBNumbering[BI->getSuccessor(1)]);
                std::stringstream ss;
                ss << std::hex << x9;
                std::string hexValue = ss.str();
                InlineAsm* Asm = InlineAsm::get(
                    FunctionType::get(Type::getVoidTy(Ctx), { Type::getInt1Ty(Ctx) },
                        false),
                    "subq $$0x0000000000000520, %rsp\n"
                    "push %rax\n"
                    "push %rbx\n"
                    "push %rcx\n"
                    "push %rdx\n"
                    "push %r8\n"
                    "push %r9\n"
                    "push %rsi\n"
                    "push %rdi\n"
                    "mov $$0x"
                        + hexValue + ", %rax\n"
                                     "xor %rbx, %rbx\n"
                                     "mov $0, %bl\n" // Move Cond to rbx
                                     "call generic_obfuscatorSpringboardFunctionCond\n",
                    "r",
                    true);
                std::vector<Value*> Args = { Cond };
                Builder.CreateCall(Asm, Args);
                // process the successor
                llvm::InlineAsm* Asm2 = llvm::InlineAsm::get(
                    llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), {},
                        false),
                    "pop %rax\n"
                    "pop %rdi\n"
                    "pop %rsi\n"
                    "pop %r9\n"
                    "pop %r8\n"
                    "pop %rdx\n"
                    "pop %rcx\n"
                    "pop %rbx\n"
                    "pop %rax\n"
                    "addq $$0x0000000000000520, %rsp\n",
                    "",
                    true);
                Builder.SetInsertPoint(BI->getSuccessor(0),
                    BI->getSuccessor(0)->getFirstInsertionPt());
                Builder.CreateCall(Asm2);
                Builder.SetInsertPoint(BI->getSuccessor(1),
                    BI->getSuccessor(1)->getFirstInsertionPt());
                Builder.CreateCall(Asm2);
            }
        } else {
            IRBuilder<> Builder(F.getContext());
            if (hasUniquePredecessor(BI->getSuccessor(0))) {
                Builder.SetInsertPoint(BI);
                long long int x9 = ComputeRegisterValue(
                    FunctionID, BBNumbering[&BB], BBNumbering[BI->getSuccessor(0)], 0);
                std::stringstream ss;
                ss << std::hex << x9;
                std::string hexValue = ss.str();
                InlineAsm* Asm = InlineAsm::get(
                    FunctionType::get(Type::getVoidTy(Ctx),
                        false),
                    "subq $$0x0000000000000520, %rsp\n"
                    "push %rax\n"
                    "push %rbx\n"
                    "push %rcx\n"
                    "push %rdx\n"
                    "push %r8\n"
                    "push %r9\n"
                    "push %rsi\n"
                    "push %rdi\n"
                    "mov $$0x"
                        + hexValue + ", %rax\n"
                                     "call generic_obfuscatorSpringboardFunction\n",
                    "",
                    true);
                Builder.CreateCall(Asm);
                llvm::InlineAsm* Asm2 = llvm::InlineAsm::get(
                    llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), {},
                        false), // 返回类型为 void，没有参数
                    "pop %rax\n"
                    "pop %rdi\n"
                    "pop %rsi\n"
                    "pop %r9\n"
                    "pop %r8\n"
                    "pop %rdx\n"
                    "pop %rcx\n"
                    "pop %rbx\n"
                    "pop %rax\n"
                    "addq $$0x0000000000000520, %rsp\n",
                    "", // 不需要输入参数
                    true // 为 true 表示不对外部调用
                );
                Builder.SetInsertPoint(BI->getSuccessor(0),
                    BI->getSuccessor(0)->getFirstInsertionPt());
                Builder.CreateCall(Asm2);
            }
        }
    }

    return 1; // 表示成功处理
}
Value* getBasicBlockAddress(Value* FunctionID, Value* BlockID,
    IRBuilder<>& Builder, Module& M)
{
    LLVMContext& Ctx = M.getContext();
    GlobalVariable* GlobalBlockArray = M.getNamedGlobal("AllFunctions_IndirectBrTargets");

    if (!GlobalBlockArray) {
        errs() << "Global block array not initialized!\n";
        return nullptr;
    }

    Value* FuncArrayPtr = Builder.CreateGEP(
        OuterArrayTy, GlobalBlockArray,
        { ConstantInt::get(Type::getInt32Ty(Ctx), 0), FunctionID });

    Value* BlockAddrPtr = Builder.CreateGEP(
        ATy, FuncArrayPtr, { ConstantInt::get(Type::getInt32Ty(Ctx), 0), BlockID });

    Value* LoadedBlockAddr = Builder.CreateLoad(Type::getInt8PtrTy(Ctx), BlockAddrPtr);

    Value* BlockAddr = Builder.CreateBitCast(LoadedBlockAddr, Type::getInt8PtrTy(Ctx));

    return BlockAddr;
}

void creategeneric_obfuscatorSpringboardFunction(Module& M)
{
    LLVMContext& Ctx = M.getContext();
    if (M.getFunction("generic_obfuscatorSpringboardFunction")) {
        return;
    }
    FunctionType* generic_obfuscatorSpringboardFunctionTy = FunctionType::get(Type::getVoidTy(Ctx), {}, false);
    Function* generic_obfuscatorSpringboardFunction = Function::Create(
        generic_obfuscatorSpringboardFunctionTy, Function::InternalLinkage, "generic_obfuscatorSpringboardFunction", &M);
    BasicBlock* EntryBB = BasicBlock::Create(Ctx, "entry", generic_obfuscatorSpringboardFunction);
    IRBuilder<> Builder(EntryBB);
    generic_obfuscatorSpringboardFunction->addFnAttr(Attribute::Naked); // 裸函数属性，不生成栈帧

    FunctionType* AsmFuncTy = FunctionType::get(Type::getInt64Ty(Ctx), {}, false);
    InlineAsm* LoadRax = InlineAsm::get(AsmFuncTy, "mov %rax, $0", "=r", true);
    Value* RaxValue = Builder.CreateCall(LoadRax);

    Value* FunctionID = Builder.CreateTrunc(Builder.CreateLShr(RaxValue, 48),
        Type::getInt16Ty(Ctx));
    Value* ID1 = Builder.CreateTrunc(Builder.CreateLShr(RaxValue, 16),
        Type::getInt16Ty(Ctx));
    Value* BBAddr = getBasicBlockAddress(FunctionID, ID1, Builder, M);
    InlineAsm* BranchAsm = InlineAsm::get(
        FunctionType::get(Type::getVoidTy(Ctx), { Type::getInt8PtrTy(Ctx) }, false),
        "push $0", // 使用 * 符号指示间接跳转
        "r", // 参数约束
        true);
    Builder.CreateCall(BranchAsm, { BBAddr });
    Builder.CreateRetVoid();
}
void creategeneric_obfuscatorSpringboardFunctionCond(Module& M)
{
    LLVMContext& Ctx = M.getContext();
    if (M.getFunction("generic_obfuscatorSpringboardFunctionCond")) {
        return;
    }
    std::string funcName = "generic_obfuscatorSpringboardFunctionCond";
    FunctionType* generic_obfuscatorSpringboardFunctionCondTy = FunctionType::get(Type::getVoidTy(Ctx), {}, false);
    Function* generic_obfuscatorSpringboardFunctionCond = Function::Create(
        generic_obfuscatorSpringboardFunctionCondTy, Function::ExternalLinkage, funcName, &M);
    generic_obfuscatorSpringboardFunctionCond->addFnAttr(Attribute::Naked);
    BasicBlock* EntryBB = BasicBlock::Create(Ctx, "entry", generic_obfuscatorSpringboardFunctionCond);
    IRBuilder<> Builder(EntryBB);

    InlineAsm* LoadRax = InlineAsm::get(FunctionType::get(Type::getInt64Ty(Ctx), {}, false),
        "movq %rax, $0", "=r", true);

    Value* RaxValue = Builder.CreateCall(LoadRax);

    Value* FunctionID = Builder.CreateTrunc(Builder.CreateLShr(RaxValue, 48),
        Type::getInt16Ty(Ctx));
    Value* TrueBlockID = Builder.CreateTrunc(Builder.CreateLShr(RaxValue, 16),
        Type::getInt16Ty(Ctx));
    Value* FalseBlockID = Builder.CreateTrunc(RaxValue, Type::getInt16Ty(Ctx));

    Value* TrueBBAddr = getBasicBlockAddress(FunctionID, TrueBlockID, Builder, M);
    Value* FalseBBAddr = getBasicBlockAddress(FunctionID, FalseBlockID, Builder, M);
    InlineAsm* LoadRbx = InlineAsm::get(FunctionType::get(Type::getInt64Ty(Ctx), {}, false),
        "mov %rbx, $0", "=r", true);
    Value* RbxValue = Builder.CreateCall(LoadRbx);
    Value* Cond = Builder.CreateICmpEQ(
        RbxValue, ConstantInt::get(Type::getInt64Ty(Ctx), 1));
    Value* TargetAddr = Builder.CreateSelect(Cond, TrueBBAddr, FalseBBAddr);
    InlineAsm* BranchAsm = InlineAsm::get(
        FunctionType::get(Type::getVoidTy(Ctx), { Type::getInt8PtrTy(Ctx) }, false),
        "push $0",
        "r",
        true);
    Builder.CreateCall(BranchAsm, { TargetAddr });
    Builder.CreateRetVoid();
}
int getBasicBlockCountIfNotSkipped(const Function& F)
{
    std::string functionName = F.getName().str();
    if (F.size() == 1) {
        return -1;
    }
    if (shouldSkip(F, branch2call)) {
        return -1;
    }
    return F.size();
}

} // namespace

PreservedAnalyses Branch2Call::run(llvm::Module& M,
    ModuleAnalysisManager& AM)
{
    LLVMContext& Ctx = M.getContext();
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    if (branch2call.model) {
        for (llvm::Function& F : M) {
            if (getBasicBlockCountIfNotSkipped(F) != -1) {
                if (maxBlockNumber < getBasicBlockCountIfNotSkipped(F) && getBasicBlockCountIfNotSkipped(F) < MaxBlockNumber) {
                    maxBlockNumber = getBasicBlockCountIfNotSkipped(F);
                }
                FunctionIndexMap[&F] = function_count++;
            }
            // llvm::outs() << "Final function count: " << function_count << "\n";
        }
        for (llvm::Function& F : M) {
            BBNumbering.clear();
            BBNumberingCount = 0;
            BBTargets.clear();

            if (getBasicBlockCountIfNotSkipped(F) == -1 || getBasicBlockCountIfNotSkipped(F) > maxBlockNumber) {
                continue;
            }
            AllFunctions_IndirectBrTargets = getIndirectTargetsMap(
                *F.getParent(), &F, AllFunctions_IndirectBrTargets);
            for (auto& BB : F) {
                auto* Terminator = BB.getTerminator();

                if (!Terminator) {
                    // llvm::errs() << "Terminator is null for basic block: " << BB.getName()
                    //              << "\n";
                    // F.print(llvm::outs());
                    continue;
                }
                auto* BI = dyn_cast<BranchInst>(Terminator);

                if (BI) {
                    unsigned FunctionID = FunctionIndexMap[&F];
                    creategeneric_obfuscatorSpringboardFunction(M);
                    creategeneric_obfuscatorSpringboardFunctionCond(M);
                    ProcessPredecessorsAndInsertFuncCall(
                        F, BB, FunctionID, F.getParent()->getFunction("generic_obfuscatorSpringboardFunction"),
                        F.getParent()->getFunction("generic_obfuscatorSpringboardFunctionCond"));
                    ++block_count;
                }
            }
            PrintSuccess("Branch2call successfully process func ", F.getName().str());
        }
    }

    return block_count > 0 ? PreservedAnalyses::none() : PreservedAnalyses::all();
}