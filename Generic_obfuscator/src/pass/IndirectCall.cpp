#include "IndirectCall.h"
#include "config.h"
#include "utils.hpp"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
using namespace llvm;
namespace KObfucator {
namespace IndirectCall {
    void process(Function& F, GlobalVariable* JumpTable, std::map<Function*, KObfucator::IndirectCallInfo>& indirectCallinfos, ArrayType* AT)
    {
        DataLayout Data = F.getParent()->getDataLayout();
        int PtrSize = Data.getTypeAllocSize(Type::getInt8Ty(F.getContext())->getPointerTo());
        Type* PtrValueType = Type::getIntNTy(F.getContext(), PtrSize * 8);
        std::vector<CallInst*> CIs;
        for (BasicBlock& BB : F) {
            for (Instruction& I : BB) {
                if (isa<CallInst>(I)) {
                    CallInst* CI = (CallInst*)&I;
                    Function* Func = CI->getCalledFunction();
                    if (Func && Func->hasExactDefinition()) {
                        CIs.push_back(CI);
                    }
                }
            }
        }
        for (CallInst* CI : CIs) {
            Type* Ty = CI->getFunctionType()->getPointerTo();
            IRBuilder<> IRB((Instruction*)CI);
            Value* KeyValue = IRB.getInt32(indirectCallinfos[CI->getCalledFunction()].key);
            KeyValue = IRB.CreateZExt(KeyValue, PtrValueType);
            Value* index = IRB.getInt32(indirectCallinfos[CI->getCalledFunction()].index);
            Value* item = IRB.CreateLoad(
                IRB.getInt8PtrTy(),
                IRB.CreateGEP(AT, JumpTable, { IRB.getInt32(0), index }));
            Value* CallPtr = IRB.CreateIntToPtr(IRB.CreateSub(item, KeyValue), Ty);
            CI->setCalledFunction(CI->getFunctionType(), CallPtr);
        }
    }
}
} // namespace KObfucator

PreservedAnalyses IndirectCall::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    bool is_processed = false;
    DataLayout Data = M.getDataLayout();
    int PtrSize = Data.getTypeAllocSize(Type::getInt8Ty(M.getContext())->getPointerTo());
    Type* PtrValueType = Type::getIntNTy(M.getContext(), PtrSize * 8);

    auto gloablName = M.getName().str() + "_FuncJmuptable";
    std::map<Function*, KObfucator::IndirectCallInfo> indirectCallinfos;
    int indirectCallinfos_count = 0;

    for (llvm::Function& F : M) {
        if (indirectCallinfos.find(&F) == indirectCallinfos.end())
            if (F.hasExactDefinition()) {
                KObfucator::IndirectCallInfo newCallInfo;
                newCallInfo.index = indirectCallinfos_count++;
                newCallInfo.key = (int)getRandomNumber();
                indirectCallinfos[&F] = newCallInfo;
            }
    }

    std::vector<Constant*> Values(indirectCallinfos_count);
    for (auto it = indirectCallinfos.begin(); it != indirectCallinfos.end(); it++) {
        Constant* CValue = ConstantExpr::getPtrToInt(
            ConstantExpr::getBitCast(it->first, it->first->getFunctionType()->getPointerTo(), false), PtrValueType, false);
        CValue = ConstantExpr::getAdd(CValue, ConstantInt::get(PtrValueType, it->second.key));
        CValue = ConstantExpr::getIntToPtr(
            CValue, Type::getInt8Ty(M.getContext())->getPointerTo());
        Values[it->second.index] = CValue;
    }
    ArrayType* AT = ArrayType::get(
        Type::getInt8Ty(M.getContext())->getPointerTo(), indirectCallinfos_count);
    GlobalVariable* JumpTable = (GlobalVariable*)M.getOrInsertGlobal(gloablName, AT);
    Constant* ValueArray = ConstantArray::get(AT, ArrayRef<Constant*>(Values));
    if (!JumpTable->hasInitializer()) {
        JumpTable->setInitializer(ValueArray);
        JumpTable->setLinkage(GlobalValue::PrivateLinkage);
    }
    if (indirect_call.model) {
        for (llvm::Function& F : M) {
            if (indirect_call.model == 2) {
                if (std::find(indirect_call.enable_function.begin(), indirect_call.enable_function.end(), F.getName()) == indirect_call.enable_function.end()) {
                    continue;
                }
            } else if (indirect_call.model == 3) {
                if (std::find(indirect_call.disable_function.begin(), indirect_call.disable_function.end(), F.getName()) != indirect_call.disable_function.end()) {
                    continue;
                }
            }
            KObfucator::IndirectCall::process(F, JumpTable, indirectCallinfos, AT);
            is_processed = true;
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}