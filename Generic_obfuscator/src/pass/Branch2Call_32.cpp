#include "../include/Branch2Call_32.h"
/* Branch 2 Call 讲条件跳转指令转为call */

#include "config.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <iomanip>
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
GlobalVariable* AllFunctions_IndirectBrTargets = nullptr;

ArrayType* OuterArrayTy = nullptr;
ArrayType* ATy = nullptr;

static int function_count = 0;
static std::map<Function*, unsigned> FunctionIndexMap;

std::random_device RandomDevice;
std::mt19937 RandomEngine(RandomDevice());

long long int ComputeRegisterValue(unsigned FunctionID, unsigned ID1,
    unsigned TrueID, unsigned FalseID)
{
    assert(FunctionID < (1 << 8) && "FunctionID exceeds 8 bits");
    assert(ID1 < (1 << 8) && "ID1 exceeds 8 bits");
    assert(TrueID < (1 << 8) && "TrueID exceeds 8 bits");
    assert(FalseID < (1 << 8) && "FalseID exceeds 8 bits");
    long long int X9Value = 0;
    X9Value |= ((long long int)FunctionID << 24); // Function ID
    X9Value |= ((long long int)ID1 << 16); // RESERVED
    X9Value |= ((long long int)TrueID << 8); // True Block ID
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
                    "subl $$0x00000520, %esp\n"
                    "push %eax\n"
                    "push %ebx\n"
                    "push %ecx\n"
                    "push %edx\n"
                    "push %edi\n"
                    "push %esi\n"
                    "mov $$0x"
                        + hexValue + ", %edi\n"
                                     "xor %ebx, %ebx\n"
                                     "mov $0, %bl\n" // Move Cond to rbx
                                     "mov %ebx, %esi\n"
                                     "call IndirectConditionalJumpFunc\n",

                    "r",
                    true);
                std::vector<Value*> Args = { Cond };
                Builder.CreateCall(Asm, Args);
                // process the successor
                llvm::InlineAsm* Asm2 = llvm::InlineAsm::get(
                    llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), {},
                        false),
                    "pop %eax\n"
                    "pop %esi\n"
                    "pop %edi\n"
                    "pop %edx\n"
                    "pop %ecx\n"
                    "pop %ebx\n"
                    "pop %eax\n"
                    "addl $$0x00000520, %esp\n",
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
                Value* X9Value = ConstantInt::get(Type::getInt32Ty(Ctx), x9);
                std::stringstream ss;
                ss << std::hex << x9;
                std::string hexValue = ss.str();
                InlineAsm* Asm = InlineAsm::get(
                    FunctionType::get(Type::getVoidTy(Ctx),
                        false),
                    "subl $$0x00000520, %esp\n"
                    "push %eax\n"
                    "push %ebx\n"
                    "push %ecx\n"
                    "push %edx\n"
                    "push %edi\n"
                    "push %esi\n"
                    "mov $$0x"
                        + hexValue + ", %edi\n"
                                     "call IndirectCallFunc\n",
                    "",
                    true);
                Builder.CreateCall(Asm);
                llvm::InlineAsm* Asm2 = llvm::InlineAsm::get(
                    llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), {},
                        false),
                    "pop %eax\n"
                    "pop %esi\n"
                    "pop %edi\n"
                    "pop %edx\n"
                    "pop %ecx\n"
                    "pop %ebx\n"
                    "pop %eax\n"
                    "addl $$0x00000520, %esp\n",
                    "",
                    true);
                Builder.SetInsertPoint(BI->getSuccessor(0),
                    BI->getSuccessor(0)->getFirstInsertionPt());
                Builder.CreateCall(Asm2);
            }
        }
    }

    return 1;
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

void createIndirectCallFunc(Module& M)
{
    LLVMContext& Ctx = M.getContext();
    if (M.getFunction("IndirectCallFunc")) {
        return;
    }
    FunctionType* IndirectCallFuncTy = FunctionType::get(Type::getVoidTy(Ctx), {}, false);
    Function* IndirectCallFunc = Function::Create(
        IndirectCallFuncTy, Function::ExternalLinkage, "IndirectCallFunc", &M);
    BasicBlock* EntryBB = BasicBlock::Create(Ctx, "entry", IndirectCallFunc);
    IRBuilder<> Builder(EntryBB);
    IndirectCallFunc->addFnAttr(Attribute::Naked); // 裸函数属性，不生成栈帧
    FunctionType* AsmFuncTy = FunctionType::get(Type::getInt32Ty(Ctx), {}, false);
    InlineAsm* LoadEax = InlineAsm::get(AsmFuncTy, "mov %edi, $0", "=r", true);
    Value* EaxValue = Builder.CreateCall(LoadEax);

    Value* FunctionID = Builder.CreateTrunc(Builder.CreateLShr(EaxValue, 24),
        Type::getInt8Ty(Ctx));
    Value* ID1 = Builder.CreateTrunc(Builder.CreateLShr(EaxValue, 8),
        Type::getInt8Ty(Ctx));
    Value* BBAddr = getBasicBlockAddress(FunctionID, ID1, Builder, M);
    InlineAsm* BranchAsm = InlineAsm::get(
        FunctionType::get(Type::getVoidTy(Ctx), { Type::getInt8PtrTy(Ctx) }, false),
        "push $0",
        "r",
        true);

    Builder.CreateCall(BranchAsm, { BBAddr });
    Builder.CreateRetVoid();
}
void createIndirectConditionalJumpFunc(Module& M)
{
    LLVMContext& Ctx = M.getContext();
    if (M.getFunction("IndirectConditionalJumpFunc")) {
        return;
    }
    std::string funcName = "IndirectConditionalJumpFunc";
    FunctionType* IndirectConditionalJumpFuncTy = FunctionType::get(Type::getVoidTy(Ctx), {}, false);
    Function* IndirectConditionalJumpFunc = Function::Create(
        IndirectConditionalJumpFuncTy, Function::ExternalLinkage, funcName, &M);
    IndirectConditionalJumpFunc->addFnAttr(Attribute::Naked); // 裸函数属性，不生成栈帧
    BasicBlock* EntryBB = BasicBlock::Create(Ctx, "entry", IndirectConditionalJumpFunc);
    IRBuilder<> Builder(EntryBB);

    InlineAsm* LoadEax = InlineAsm::get(FunctionType::get(Type::getInt32Ty(Ctx), {}, false),
        "movl %edi, $0", "=r", true);
    InlineAsm* LoadEbx = InlineAsm::get(FunctionType::get(Type::getInt32Ty(Ctx), {}, false),
        "mov %esi, $0", "=r", true);
    Value* EaxValue = Builder.CreateCall(LoadEax);
    Value* EbxValue = Builder.CreateCall(LoadEbx);
    
    Value* FunctionID = Builder.CreateTrunc(Builder.CreateLShr(EaxValue, 24),
        Type::getInt8Ty(Ctx));
    Value* TrueBlockID = Builder.CreateTrunc(Builder.CreateLShr(EaxValue, 8),
        Type::getInt8Ty(Ctx));
    Value* FalseBlockID = Builder.CreateTrunc(EaxValue, Type::getInt8Ty(Ctx));

 
    Value* TrueBBAddr = getBasicBlockAddress(FunctionID, TrueBlockID, Builder, M);
    Value* FalseBBAddr = getBasicBlockAddress(FunctionID, FalseBlockID, Builder, M);
    Value* Cond = Builder.CreateICmpEQ(
        EbxValue, ConstantInt::get(Type::getInt32Ty(Ctx), 1));
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
    if (functionName == "IndirectConditionalJumpFunc" || functionName == "IndirectCallFunc") {
        return -1;
    }

    if (F.empty() || F.hasLinkOnceLinkage() || F.getSection() == ".text.startup") {
        return -1;
    }

    if (branch2call_32.model == 2) {
        if (std::find(branch2call_32.enable_function.begin(),
                branch2call_32.enable_function.end(),
                functionName)
            == branch2call_32.enable_function.end()) {
            return -1;
        }
    } else if (branch2call_32.model == 3) {
        if (std::find(branch2call_32.disable_function.begin(),
                branch2call_32.disable_function.end(),
                functionName)
            != branch2call_32.disable_function.end()) {
            return -1;
        }
    }
    return F.size();
}

} // namespace

PreservedAnalyses Branch2Call_32::run(llvm::Module& M,
    ModuleAnalysisManager& AM)
{
    LLVMContext& Ctx = M.getContext();
    // DataLayout Data = M.getDataLayout();
    //   int PtrSize =
    //       Data.getTypeAllocSize(Type::getInt8Ty(F.getContext())->getPointerTo());
    //   Type *PtrValueType = Type::getIntNTy(F.getContext(), PtrSize * 8);
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    if (branch2call_32.model) {
        for (llvm::Function& F : M) {
            if (getBasicBlockCountIfNotSkipped(F) != -1) {
                if (maxBlockNumber < getBasicBlockCountIfNotSkipped(F) && getBasicBlockCountIfNotSkipped(F) < MaxBlockNumber) {
                    maxBlockNumber = getBasicBlockCountIfNotSkipped(F);
                }
                FunctionIndexMap[&F] = function_count++;
            }
            llvm::outs() << "Final function count: " << function_count << "\n";
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
                    llvm::errs() << "Terminator is null for basic block: " << BB.getName()
                                 << "\n";
                    F.print(llvm::outs());
                    continue;
                }
                auto* BI = dyn_cast<BranchInst>(Terminator);

                if (BI) {
                    unsigned FunctionID = FunctionIndexMap[&F];
                    createIndirectCallFunc(M);
                    createIndirectConditionalJumpFunc(M);
                    ProcessPredecessorsAndInsertFuncCall(
                        F, BB, FunctionID, F.getParent()->getFunction("IndirectCallFunc"),
                        F.getParent()->getFunction("IndirectConditionalJumpFunc"));
                    ++block_count;
                }
            }
        }
    }

    return block_count > 0 ? PreservedAnalyses::none() : PreservedAnalyses::all();
}