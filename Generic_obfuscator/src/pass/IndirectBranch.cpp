#include "IndirectBranch.h"
#include "config.h"
#include "utils.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include <cstdio>
#include <string>
#include <vector>
#include "Log.hpp"
using namespace llvm;
namespace Generic_obfuscator {
namespace IndirectBranch {

    void process(Function& F)
    {
        DataLayout Data = F.getParent()->getDataLayout();
        int PtrSize = Data.getTypeAllocSize(Type::getInt8Ty(F.getContext())->getPointerTo());
        Type* PtrValueType = Type::getIntNTy(F.getContext(), PtrSize * 8);
        auto module = F.getParent();

        std::map<BasicBlock*, Generic_obfuscator::IndirectBBinfo> indirectBBinfos;
        int indirectBBs_count = 0;
        std::vector<BranchInst*> branchInsts;
        auto gloablName = F.getName().str() + "_Jmuptable";

        for (BasicBlock& BB : F) {
            for (Instruction& I : BB) {
                if (isa<BranchInst>(I)) {
                    BranchInst* Br = (BranchInst*)(&I);
                    branchInsts.push_back(Br);
                    if (Br->isConditional()) {
                        BasicBlock* TrueBB = Br->getSuccessor(0);
                        BasicBlock* FalseBB = Br->getSuccessor(1);
                        if (indirectBBinfos.find(TrueBB) == indirectBBinfos.end()) {
                            IndirectBBinfo newBBinfo;
                            newBBinfo.index = indirectBBs_count++;
                            newBBinfo.key = (int)getRandomNumber();
                            indirectBBinfos[TrueBB] = newBBinfo;
                        }
                        if (indirectBBinfos.find(FalseBB) == indirectBBinfos.end()) {
                            IndirectBBinfo newBBinfo;
                            newBBinfo.index = indirectBBs_count++;
                            newBBinfo.key = (int)getRandomNumber();
                            indirectBBinfos[FalseBB] = newBBinfo;
                        }
                    } else {
                        BasicBlock* BB = Br->getSuccessor(0);
                        if (indirectBBinfos.find(BB) == indirectBBinfos.end()) {
                            IndirectBBinfo newBBinfo;
                            newBBinfo.index = indirectBBs_count++;
                            newBBinfo.key = (int)getRandomNumber();
                            indirectBBinfos[BB] = newBBinfo;
                        }
                    }
                }
            }
        }
        std::vector<Constant*> Values(indirectBBs_count);
        for (auto it = indirectBBinfos.begin(); it != indirectBBinfos.end(); it++) {
            BlockAddress* BA = BlockAddress::get(it->first);
            Constant* CValue = ConstantExpr::getPtrToInt(BA, PtrValueType);
            CValue = ConstantExpr::getAdd(
                CValue, ConstantInt::get(PtrValueType, it->second.key));
            CValue = ConstantExpr::getIntToPtr(
                CValue, Type::getInt8Ty(F.getContext())->getPointerTo());
            Values[it->second.index] = CValue;
        }
        ArrayType* AT = ArrayType::get(
            Type::getInt8Ty(F.getContext())->getPointerTo(), indirectBBs_count);
        GlobalVariable* JumpTable = (GlobalVariable*)module->getOrInsertGlobal(gloablName, AT);
        Constant* ValueArray = ConstantArray::get(AT, ArrayRef<Constant*>(Values));
        if (!JumpTable->hasInitializer()) {
            JumpTable->setInitializer(ValueArray);
            JumpTable->setLinkage(GlobalValue::PrivateLinkage);
        }
        for (BranchInst* Br : branchInsts) {
            IRBuilder<> IRB(Br);
            if (Br->isConditional()) {
                BasicBlock* TrueBB = Br->getSuccessor(0);
                BasicBlock* FalseBB = Br->getSuccessor(1);
                Value* Cond = Br->getCondition();
                Value* Index = IRB.CreateSelect(Cond, IRB.getInt32(indirectBBinfos[TrueBB].index),
                    IRB.getInt32(indirectBBinfos[FalseBB].index));
                Value* Item = IRB.CreateLoad(
                    IRB.getInt8PtrTy(),
                    IRB.CreateGEP(AT, JumpTable, { IRB.getInt32(0), Index }));

                Value* Key = IRB.CreateSelect(Cond, IRB.getIntN(PtrSize * 8, indirectBBinfos[TrueBB].key),
                    IRB.getIntN(PtrSize * 8, indirectBBinfos[FalseBB].key));
                Value* Addr = IRB.CreateIntToPtr(
                    IRB.CreateSub(IRB.CreatePtrToInt(Item, PtrValueType), Key),
                    IRB.getInt8PtrTy());

                IndirectBrInst* IBR = IRB.CreateIndirectBr(Addr);
                IBR->addDestination(TrueBB);
                IBR->addDestination(FalseBB);
                Br->eraseFromParent();
            } else {
                BasicBlock* BB = Br->getSuccessor(0);
                Value* Item = IRB.CreateLoad(
                    IRB.getInt8PtrTy(),
                    IRB.CreateGEP(AT, JumpTable,
                        { IRB.getInt32(0), IRB.getInt32(indirectBBinfos[BB].index) }));
                Value* Key = IRB.getIntN(PtrSize * 8, indirectBBinfos[BB].key);
                Value* Addr = IRB.CreateIntToPtr(
                    IRB.CreateSub(IRB.CreatePtrToInt(Item, PtrValueType), Key),
                    IRB.getInt8PtrTy());
                IndirectBrInst* IBR = IRB.CreateIndirectBr(Addr);
                IBR->addDestination(BB);
                Br->eraseFromParent();
            }
        }
    }
}
} // namespace Generic_obfuscator
PreservedAnalyses IndirectBranch::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    bool is_processed = false;
    if (indirectBranch.model) {
        for (llvm::Function& F : M) {
            if (shouldSkip(F, indirectBranch)) {
                continue;
            }
            Generic_obfuscator::IndirectBranch::process(F);
            is_processed = true;
            PrintSuccess("IndirectBranch successfully process func ", F.getName().str());
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}