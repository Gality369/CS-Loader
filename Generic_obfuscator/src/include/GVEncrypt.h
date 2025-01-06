#ifndef LLVM_Kotoamatsukami_GVEncrypt_H
#define LLVM_Kotoamatsukami_GVEncrypt_H
// 现在仅适配了AArch64架构 因为不同架构之间汇编不一样
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Passes/PassBuilder.h"
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace llvm;
using namespace std;
namespace llvm {
class GVEncrypt : public PassInfoMixin<GVEncrypt> {
public:
    PreservedAnalyses run(Module& M, ModuleAnalysisManager& AM);
    // 保证不被跳过
    static bool isRequired() { return true; }
};
}
namespace Kotoamatsukami {
namespace GVEncrypt {
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
        void traversePath(BasicBlock* BB, std::set<GlobalVariable *> &gv_set);

        std::size_t hashSet(const std::set<GlobalVariable*>& gv_set);

    public:
        // 构造函数
        TaintAnalysis() = default;

        // 析构函数
        ~TaintAnalysis() = default;

        // 分析函数中的污点
        void analyzeFunction(Function& F);

        void analyzeFunctionFlowSensitive(Function& F);

        // 获取共享变量访问指令集合
        const std::set<Instruction*>& getUnnecessarySet() const;

        // 打印共享变量访问指令集合
        void printUnnecessarySet() const;

        
    }; // Class TainAnalysis end

    bool encryptGV(llvm::Function *F, Function* decryptFunction);
    struct GVInfo {
      int index;
      uint8_t key;
      int len;
    };
    bool shouldSkip(GlobalVariable &GV);
    Function *defineDecryptFunction(Module* M, GlobalVariable* GVIsDecrypted);
} // namespace GVEncrypt
} // namespace KOtoamatsukami
#endif