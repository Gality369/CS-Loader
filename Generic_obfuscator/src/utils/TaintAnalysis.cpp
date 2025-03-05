#include "TaintAnalysis.h"

std::size_t Generic_obfuscator::TaintAnalysis::hashSet(const std::set<GlobalVariable*>& gv_set)
{
    std::size_t hashValue = 0;

    for (const auto& val : gv_set) {
        hashValue ^= reinterpret_cast<std::size_t>(val);
    }

    return hashValue;
}

void Generic_obfuscator::TaintAnalysis::traversePath(BasicBlock* BB, std::set<GlobalVariable*>& gv_set, std::set<GlobalVariable *>& needEncGV)
{
    if (!BB || visitedBlocks.count(BB)) {
        return;
    }
    if (bb_2_gv_set_hash[BB].count(hashSet(gv_set))) {
        return;
    }
    bb_2_gv_set_hash[BB].insert(hashSet(gv_set));
    visitedBlocks.insert(BB);
    currentPath.push_back(BB);
    for (auto& I : *BB) {
        std::set<Value*> opValSet;
        if (auto* op = dyn_cast<Instruction>(&I)) {
            for (unsigned i = 0; i < op->getNumOperands(); ++i) {
                auto* operand = op->getOperand(i);
                if (operand)
                    opValSet.insert(operand);
            }
        }
        for (auto* opVal : opValSet) {
            if (auto* GV = dyn_cast<GlobalVariable>(opVal)) {
                if (needEncGV.find(GV) != needEncGV.end() && gv_set.find(GV) != gv_set.end()) {
                    unnecessarySet.insert(&I);
                } else {
                    gv_set.insert(GV);
                    gv_set_each_path.insert(GV);
                }
            }
        }
    }
    for (auto* Succ : successors(BB)) {
        traversePath(Succ, gv_set,needEncGV);
    }

    for (auto Val : gv_set_each_path) {
        gv_set.erase(Val);
    }
    gv_set_each_path.clear();
    currentPath.pop_back();
}
void Generic_obfuscator::TaintAnalysis::analyzeFunctionFlowSensitive(Function& F, std::set<GlobalVariable *>& needEncGV)
{
    gv_set.clear();
    gv_set_each_path.clear();
    unnecessarySet.clear();
    if (F.empty())
        return;
    Module* M = F.getParent();
    if (!M)
        return;

    BasicBlock* entryBB = &F.getEntryBlock();
    if (!entryBB)
        return;

    traversePath(entryBB, gv_set,needEncGV);
}

const std::set<Instruction*>& Generic_obfuscator::TaintAnalysis::getUnnecessarySet() const
{
    return unnecessarySet;
}

void Generic_obfuscator::TaintAnalysis::printUnnecessarySet() const
{
    for (auto* I : unnecessarySet) {
        errs() << "Accessed Instruction: " << *I << "\n";
    }
}

