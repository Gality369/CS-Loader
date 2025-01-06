// This code is adapted from the Pluto project (https://github.com/DreamSoule/ollvm17)
// Source file: https://github.com/DreamSoule/ollvm17/blob/main/llvm-project/llvm/lib/Passes/Obfuscation/Flattening.cpp

#include "Flatten.h"
#include "config.h"
#include "utils.hpp"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
using namespace llvm;
using std::vector;
namespace Kotoamatsukami {
namespace Flatten {
    bool flatten(Function& F)
    {
        if (F.size() <= 1) {
            return false;
        }
        vector<BasicBlock*> origBB;
        for (BasicBlock& BB : F) {
            origBB.push_back(&BB);
        }
        origBB.erase(origBB.begin());
        BasicBlock& entryBB = F.getEntryBlock();
        bool bEntryBB_isConditional = false;
        if (BranchInst* br = dyn_cast<BranchInst>(entryBB.getTerminator())) {
            if (br->isConditional()) {
                BasicBlock* newBB = entryBB.splitBasicBlock(br, "newBB");
                origBB.insert(origBB.begin(), newBB);
                bEntryBB_isConditional = true;
            }
        }
        BasicBlock* dispatchBB = BasicBlock::Create(F.getContext(), "dispatchBB", &F, &entryBB);
        BasicBlock* returnBB = BasicBlock::Create(F.getContext(), "returnBB", &F, &entryBB);
        BranchInst::Create(dispatchBB, returnBB);
        entryBB.moveBefore(dispatchBB);
        if (bEntryBB_isConditional) {
            entryBB.getTerminator()->eraseFromParent();
        }
        BranchInst* brDispatchBB = BranchInst::Create(dispatchBB, &entryBB);
        int randNumCase = rand();
        AllocaInst* swVarPtr = new AllocaInst(Type::getInt32Ty(F.getContext()), 0, "swVar.ptr", brDispatchBB);
        new StoreInst(ConstantInt::get(Type::getInt32Ty(F.getContext()), randNumCase), swVarPtr, brDispatchBB);
        LoadInst* swVar = new LoadInst(Type::getInt32Ty(F.getContext()), swVarPtr, "swVar", false, dispatchBB);
        BasicBlock* swDefault = BasicBlock::Create(F.getContext(), "swDefault", &F, returnBB);
        BranchInst::Create(returnBB, swDefault);
        SwitchInst* swInst = SwitchInst::Create(swVar, swDefault, 0, dispatchBB);
        for (BasicBlock* BB : origBB) {
            BB->moveBefore(returnBB);
            swInst->addCase(ConstantInt::get(Type::getInt32Ty(F.getContext()), randNumCase), BB);
            randNumCase = rand();
        }

        for (BasicBlock* BB : origBB) {
            if (BB->getTerminator()->getNumSuccessors() == 0) {
                continue;
            }
            else if (BB->getTerminator()->getNumSuccessors() == 1) {
                BasicBlock* sucBB = BB->getTerminator()->getSuccessor(0);
                if (bEntryBB_isConditional) {
                    entryBB.getTerminator()->eraseFromParent();
                }
                ConstantInt* numCase = swInst->findCaseDest(sucBB);
                new StoreInst(numCase, swVarPtr, BB);
                BranchInst::Create(returnBB, BB);
            }
            else if (BB->getTerminator()->getNumSuccessors() == 2) {
                BranchInst* br = dyn_cast<BranchInst>(BB->getTerminator());
                if (!br) {
                    continue;
                }
                if (!br->isConditional()) {
                    continue;
                }
                ConstantInt* numCaseTrue = swInst->findCaseDest(BB->getTerminator()->getSuccessor(0));
                ConstantInt* numCaseFalse = swInst->findCaseDest(BB->getTerminator()->getSuccessor(1));
                SelectInst* sel = SelectInst::Create(br->getCondition(), numCaseTrue, numCaseFalse, "", BB->getTerminator());
                BB->getTerminator()->eraseFromParent();
                new StoreInst(sel, swVarPtr, BB);
                BranchInst::Create(returnBB, BB);
            }
        }
        demoteRegisters(&F);
        return true;
    }
}
}

PreservedAnalyses Flatten::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/Kotoamatsukami/config/config.json");
    bool is_processed = false;
    if (flatten.model) {
        for (llvm::Function& F : M) {
            if (flatten.model == 2) {
                if (std::find(flatten.enable_function.begin(), flatten.enable_function.end(), F.getName()) == flatten.enable_function.end()) {
                    continue;
                }
            } else if (flatten.model == 3) {
                if (std::find(flatten.disable_function.begin(), flatten.disable_function.end(), F.getName()) != flatten.disable_function.end()) {
                    continue;
                }
            }
            if (!F.hasExactDefinition()) {
                continue;
            }
            Kotoamatsukami::Flatten::flatten(F);
            is_processed = true;
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}