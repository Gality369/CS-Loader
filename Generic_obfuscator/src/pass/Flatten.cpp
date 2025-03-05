#include "Flatten.h"
#include "Log.hpp"
#include "config.h"
#include "utils.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include <cstdlib>
#include <iterator>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;
namespace Generic_obfuscator {
namespace Flatten {
    bool flatteningLinearBlocks(Function& F, std::vector<llvm::BasicBlock*> linearPath)
    {
        BasicBlock* entryBB = linearPath[0];
        BasicBlock* dispatchBB = BasicBlock::Create(F.getContext(), "dispatchBB", &F, entryBB);
        BasicBlock* returnBB = BasicBlock::Create(F.getContext(), "returnBB", &F, entryBB);
        std::unordered_map<BasicBlock*, int> bb2Randnuml;

        linearPath.erase(linearPath.begin());
        // Prevent random number duplication
        std::vector<int> randVec = generateUniqueRandomNumbers(520, 1314, linearPath.size());
        for (unsigned int i = 0; i < linearPath.size(); i++) {
            BasicBlock* BB = linearPath[i];
            BB->moveBefore(returnBB);
            bb2Randnuml[BB] = randVec[i];
        }

        // for (int i = 0; i < linearPath.size(); i++) {
        //     linearPath[i]->print(errs());
        // }
        // entryBB
        entryBB->moveBefore(dispatchBB);
        IRBuilder<> builder(entryBB);
        builder.SetInsertPoint(&*entryBB->getFirstInsertionPt());
        AllocaInst* swVarPtr = builder.CreateAlloca(Type::getInt32Ty(F.getContext()), nullptr, "linear.1swVar.ptr");
        builder.CreateStore(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnuml[linearPath[0]]), swVarPtr);
        entryBB->getTerminator()->eraseFromParent();
        builder.SetInsertPoint(entryBB);
        builder.CreateBr(dispatchBB);
        // returnBB
        builder.SetInsertPoint(returnBB);
        builder.CreateBr(dispatchBB);

        // dispatchBB
        builder.SetInsertPoint(dispatchBB);
        LoadInst* swVar = builder.CreateLoad(Type::getInt32Ty(F.getContext()), swVarPtr);
        SwitchInst* swInst = builder.CreateSwitch(swVar, returnBB, 0);

        for (unsigned int i = 0; i < linearPath.size(); i++) {
            swInst->addCase(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnuml[linearPath[i]]), linearPath[i]);
        }

        for (unsigned int i = 0; i < linearPath.size() - 1; i++) {
            BasicBlock* BB = linearPath[i];
            BB->moveBefore(returnBB);
            builder.SetInsertPoint(BB->getTerminator());
            builder.CreateStore(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnuml[BB->getSingleSuccessor()]), swVarPtr);
            BB->getTerminator()->eraseFromParent();
            builder.SetInsertPoint(BB);
            builder.CreateBr(returnBB);
        }

        return true;
    }
    bool flattening(Function& F)
    {
        if (F.size() <= 1) {
            return false;
        }
        SmallVector<BasicBlock*, 1024> origBB;
        for (BasicBlock& BB : F) {
            origBB.push_back(&BB);
        }
        origBB.erase(origBB.begin());

        // process conditional entryBB
        BasicBlock& entryBB = F.getEntryBlock();
        bool bEntryBB_isConditional = false;
        BasicBlock* newBB = nullptr;
        if (BranchInst* br = dyn_cast<BranchInst>(entryBB.getTerminator())) {
            if (br->isConditional()) {
                newBB = entryBB.splitBasicBlock(br, "newBB");
                origBB.insert(origBB.begin(), newBB);
                bEntryBB_isConditional = true;
            }
        }
        // Prevent random number duplication
        std::unordered_map<BasicBlock*, int> bb2Randnum;
        std::vector<int> randVec = generateUniqueRandomNumbers(520, 1314, origBB.size());
        for (unsigned int i = 0; i < origBB.size(); i++) {
            BasicBlock* BB = origBB[i];
            bb2Randnum[BB] = randVec[i];
        }

        BasicBlock* dispatchBB = BasicBlock::Create(F.getContext(), "dispatchBB", &F, &entryBB);
        BasicBlock* returnBB = BasicBlock::Create(F.getContext(), "returnBB", &F, &entryBB);

        // entryBB
        entryBB.moveBefore(dispatchBB);
        IRBuilder<> builder(&entryBB);
        builder.SetInsertPoint(&*entryBB.getFirstInsertionPt());
        AllocaInst* swVarPtr = builder.CreateAlloca(Type::getInt32Ty(F.getContext()), nullptr, "swVar.ptr");
        if (bEntryBB_isConditional) {
            builder.CreateStore(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[newBB]), swVarPtr);
        } else {
            builder.CreateStore(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[entryBB.getTerminator()->getSuccessor(0)]), swVarPtr);
        }
        if (entryBB.getTerminator())
            entryBB.getTerminator()->eraseFromParent();
        builder.SetInsertPoint(&entryBB);
        builder.CreateBr(dispatchBB);

        // swDefault
        BasicBlock* swDefault = BasicBlock::Create(F.getContext(), "swDefault", &F, returnBB);
        builder.SetInsertPoint(swDefault);
        builder.CreateBr(returnBB);

        // disaptchBB
        builder.SetInsertPoint(dispatchBB);
        LoadInst* swVar = builder.CreateLoad(Type::getInt32Ty(F.getContext()), swVarPtr);
        SwitchInst* swInst = builder.CreateSwitch(swVar, swDefault, 0);

        // returnBB
        builder.SetInsertPoint(returnBB);
        builder.CreateBr(dispatchBB);

        // add cases to switch
        for (BasicBlock* BB : origBB) {
            BB->moveBefore(returnBB);
            swInst->addCase(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[BB]), BB);
        }

        // process BBs in origBB
        for (BasicBlock* BB : origBB) {
            if (BB->getTerminator()->getNumSuccessors() == 0) {
                continue;
            } else if (BB->getTerminator()->getNumSuccessors() == 1) {
                BasicBlock* sucBB = BB->getTerminator()->getSuccessor(0);
                BB->getTerminator()->eraseFromParent();
                builder.SetInsertPoint(BB);
                builder.CreateStore(ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[sucBB]), swVarPtr);
                builder.CreateBr(returnBB);
            } else if (BB->getTerminator()->getNumSuccessors() == 2) {
                BranchInst* br = dyn_cast<BranchInst>(BB->getTerminator());
                if (!br) {
                    continue;
                }
                if (!br->isConditional()) {
                    continue;
                }
                Value* cond = br->getCondition();
                BasicBlock* trueBB = br->getSuccessor(0);
                BasicBlock* falseBB = br->getSuccessor(1);
                builder.SetInsertPoint(&*BB->getTerminator());
                Value* selValue = builder.CreateSelect(cond, ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[trueBB]), ConstantInt::get(Type::getInt32Ty(F.getContext()), bb2Randnum[falseBB]), "select");
                builder.CreateStore(selValue, swVarPtr);
                BB->getTerminator()->eraseFromParent();
                builder.SetInsertPoint(BB);
                builder.CreateBr(returnBB);
            }
        }

        // process linear paths
        for (auto it = origBB.begin(); it != origBB.end();) {
            BasicBlock* BB = *it;
            if (BB->size() > 4) {
                if (rand() % 100 < 100) {
                    if (containsPHI(BB)) {
                        ++it;
                        continue;
                    
                    }
                    std::vector<BasicBlock*> splitBBs = splitBasicBlockRandomly(BB, 4);
                    flatteningLinearBlocks(F, splitBBs);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
        demoteRegisters(&F);
        return true;
    }
}
}

PreservedAnalyses Flatten::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    bool is_processed = false;
    if (flatten.model) {
        for (llvm::Function& F : M) {
            if (shouldSkip(F, flatten)) {
                continue;
            }
            Generic_obfuscator::Flatten::flattening(F);
            is_processed = true;
            PrintSuccess("Flattening successfully ", F.getName().str());
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}