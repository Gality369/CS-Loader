#include "SplitBasicBlock.h"
#include <algorithm>
#include <random>
#include <set>
#include <vector>
#include "config.h"
#include "utils.hpp"
using namespace llvm;

bool SplitBasicBlock::containsPHI(BasicBlock *BB) {
    for (Instruction &I : *BB) {
        if (isa<PHINode>(&I)) {
            return true;
        }
    }
    return false;
}

void SplitBasicBlock::split(Function *F, int splitNumber) {
    std::set<BasicBlock *> processedBB;
    std::set<BasicBlock *> createBB;
    std::vector<BasicBlock *> origBB;
    std::vector<int> splitPointVec;
    std::random_device rd;
    std::default_random_engine e(rd());

    llvm::outs() << "Processing Function: " << F->getName() << "\n";
    for (BasicBlock &BB : *F) {
        origBB.push_back(&BB);
    }
    for (BasicBlock *BB : origBB) {
        int BBsize = BB->size();
        if (BBsize < 2 || containsPHI(BB)) {
            llvm::outs() << "Skipping Basic Block (too small or contains PHI): " << BB->getName() << "\n";
            continue;
        }
        if ((size_t)splitNumber >= BBsize) {
            splitNumber = BBsize - 1;
        }
        // Get split points
        splitPointVec.clear();
        int temp = 0;
        while (temp < splitNumber) {
            int random_number = rand() % BBsize;
            if (std::find(splitPointVec.begin(), splitPointVec.end(), random_number) == splitPointVec.end()) {
                splitPointVec.push_back(random_number);
                temp++;
            }
        }
        std::sort(splitPointVec.begin(), splitPointVec.end());

        llvm::outs() << "Split points: ";
        for (int pt : splitPointVec) {
            llvm::outs() << pt << " ";
        }
        llvm::outs() << "\n";

        BasicBlock::iterator it = BB->begin();
        BasicBlock *toSplit = BB;
        int last = 0;
        for (int i = 0; i < splitNumber; ++i) {
            if (toSplit->size() < 2) {
                continue;
            }
            for (int j = 0; j < splitPointVec[i] - last; ++j) {
                ++it;
            }
            last = splitPointVec[i];
            BasicBlock *newBB = toSplit->splitBasicBlock(it, toSplit->getName() + ".split");
            toSplit = newBB;
        }   
    }
}

PreservedAnalyses SplitBasicBlock::run(Module &M, ModuleAnalysisManager &AM) {
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    
    if (SplitBasicBlocks.model){
        for (llvm::Function &F : M) {
            if(SplitBasicBlocks.model == 2){
                if(std::find(SplitBasicBlocks.enable_function.begin(),SplitBasicBlocks.enable_function.end(),F.getName()) == SplitBasicBlocks.enable_function.end()){
                    continue;                    
                }
            }else if (SplitBasicBlocks.model == 3)
            {                
                if(std::find(SplitBasicBlocks.disable_function.begin(),SplitBasicBlocks.disable_function.end(),F.getName()) != SplitBasicBlocks.disable_function.end()){
                    continue;                    
                }
            }
            
            split(&F, SplitBasicBlocks.op1);
        }
    }

    
    llvm::outs() << "Splitting completed.\n";
    return PreservedAnalyses::none();
}
