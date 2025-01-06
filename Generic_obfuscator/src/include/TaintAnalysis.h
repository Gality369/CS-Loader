#ifndef LLVM_Kotoamatsukami_TaintAnalysis_H
#define LLVM_Kotoamatsukami_TaintAnalysis_H
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instruction.h"
#include <cstdio>
#include <map>
#include <set>
#include <vector>
using namespace llvm;
using namespace std;
namespace Kotoamatsukami {

class TaintAnalysis {
private:
    std::set<GlobalVariable*> gv_set_each_path;
    std::set<GlobalVariable*> gv_set;
    Value* var_set_end = nullptr;
    std::map<BasicBlock*, std::set<std::size_t>> bb_2_gv_set_hash;
    // 共享变量访问的指令集合
    std::set<Instruction*> unnecessarySet;
    std::vector<BasicBlock*> currentPath;
    std::set<BasicBlock*> visitedBlocks;
    void traversePath(BasicBlock* BB, std::set<GlobalVariable*>& gv_set, std::set<GlobalVariable *>& needEncGV);

    std::size_t hashSet(const std::set<GlobalVariable*>& gv_set);

public:
    // 构造函数
    TaintAnalysis() = default;

    // 析构函数
    ~TaintAnalysis() = default;

    void analyzeFunctionFlowSensitive(Function& F, std::set<GlobalVariable *>& needEncGV);

    // 获取共享变量访问指令集合
    const std::set<Instruction*>& getUnnecessarySet() const;

    // 打印共享变量访问指令集合
    void printUnnecessarySet() const;

}; // Class TainAnalysis end
} // namespace KOtoamatsukami
#endif