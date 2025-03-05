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

    for (BasicBlock &BB : *F) {
        origBB.push_back(&BB);
    }
    for (BasicBlock *BB : origBB) {
        int BBsize = BB->size();
        if (BBsize < 2 || containsPHI(BB)) {
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
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    if (splitBasicBlocks.model){
        for (llvm::Function &F : M) {
            if (shouldSkip(F, splitBasicBlocks)){
                continue;
            }            
            split(&F, splitBasicBlocks.op1);
        }
    }
    return PreservedAnalyses::none();
}
