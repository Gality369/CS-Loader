// This code is adapted from the Pluto project (https://github.com/bluesadi/Pluto)
// Source file: https://github.com/bluesadi/Pluto/blob/kanxue/Transforms/src/Substitution.cpp
// I just adapted it to the LLVM-17 and the LLVM New Pass
#include "Substitution.h"
#include "config.h"
#include "utils.hpp"
#include "Log.hpp"

using namespace llvm;
using std::vector;

#define NUMBER_ADD_SUBST 4
#define NUMBER_SUB_SUBST 3
#define NUMBER_AND_SUBST 2
#define NUMBER_OR_SUBST 2
#define NUMBER_XOR_SUBST 2

// 混淆次数，混淆次数越多混淆结果越复杂
int sub_times = 3;
namespace Generic_obfuscator {
namespace Substitution {

    void substitute(BinaryOperator* BI)
    {
        bool flag = true;
        switch (BI->getOpcode()) {
        case BinaryOperator::Add:
            substituteAdd(BI);
            break;
        case BinaryOperator::Sub:
            substituteSub(BI);
            break;
        case BinaryOperator::And:
            substituteAnd(BI);
            break;
        case BinaryOperator::Or:
            substituteOr(BI);
            break;
        case BinaryOperator::Xor:
            substituteXor(BI);
            break;
        default:
            flag = false;
            break;
        }
        if (flag) {
            BI->eraseFromParent();
        }
    }

    void substituteAdd(BinaryOperator* BI)
    {
        int choice = rand() % NUMBER_ADD_SUBST;
        switch (choice) {
        case 0:
            addNeg(BI);
            break;
        case 1:
            addDoubleNeg(BI);
            break;
        case 2:
            addRand(BI);
            break;
        case 3:
            addRand2(BI);
            break;
        default:
            break;
        }
    }

    void addNeg(BinaryOperator* BI)
    {
        BinaryOperator* op;
        op = BinaryOperator::CreateNeg(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateSub(BI->getOperand(0), op, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void addDoubleNeg(BinaryOperator* BI)
    {
        BinaryOperator *op, *op1, *op2;
        op1 = BinaryOperator::CreateNeg(BI->getOperand(0), "", BI);
        op2 = BinaryOperator::CreateNeg(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateAdd(op1, op2, "", BI);
        op = BinaryOperator::CreateNeg(op, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void addRand(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator* op;
        op = BinaryOperator::CreateAdd(BI->getOperand(0), r, "", BI);
        op = BinaryOperator::CreateAdd(op, BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateSub(op, r, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void addRand2(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1, *op2;
        op = BinaryOperator::CreateSub(BI->getOperand(0), r, "", BI);
        op = BinaryOperator::CreateAdd(op, BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateAdd(op, r, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void substituteSub(BinaryOperator* BI)
    {
        int choice = rand() % NUMBER_SUB_SUBST;
        switch (choice) {
        case 0:
            subNeg(BI);
            break;
        case 1:
            subRand(BI);
            break;
        case 2:
            subRand2(BI);
            break;
        default:
            break;
        }
    }

    void subNeg(BinaryOperator* BI)
    {
        BinaryOperator* op;
        op = BinaryOperator::CreateNeg(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateAdd(BI->getOperand(0), op, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void subRand(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1, *op2;
        op = BinaryOperator::CreateAdd(BI->getOperand(0), r, "", BI);
        op = BinaryOperator::CreateSub(op, BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateSub(op, r, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void subRand2(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1, *op2;
        op = BinaryOperator::CreateSub(BI->getOperand(0), r, "", BI);
        op = BinaryOperator::CreateSub(op, BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateAdd(op, r, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void substituteXor(BinaryOperator* BI)
    {
        int choice = rand() % NUMBER_XOR_SUBST;
        switch (choice) {
        case 0:
            xorSubstitute(BI);
            break;
        case 1:
            xorSubstituteRand(BI);
            break;
        default:
            break;
        }
    }

    void xorSubstitute(BinaryOperator* BI)
    {
        BinaryOperator *op, *op1, *op2, *op3;
        op1 = BinaryOperator::CreateNot(BI->getOperand(0), "", BI);
        op1 = BinaryOperator::CreateAnd(op1, BI->getOperand(1), "", BI);
        op2 = BinaryOperator::CreateNot(BI->getOperand(1), "", BI);
        op2 = BinaryOperator::CreateAnd(BI->getOperand(0), op2, "", BI);
        op = BinaryOperator::CreateOr(op1, op2, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void xorSubstituteRand(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1, *op2, *op3;
        op1 = BinaryOperator::CreateNot(BI->getOperand(0), "", BI);
        op1 = BinaryOperator::CreateAnd(op1, r, "", BI);
        op2 = BinaryOperator::CreateNot(r, "", BI);
        op2 = BinaryOperator::CreateAnd(BI->getOperand(0), op2, "", BI);
        op = BinaryOperator::CreateOr(op1, op2, "", BI);
        op1 = BinaryOperator::CreateNot(BI->getOperand(1), "", BI);
        op1 = BinaryOperator::CreateAnd(op1, r, "", BI);
        op2 = BinaryOperator::CreateNot(r, "", BI);
        op2 = BinaryOperator::CreateAnd(BI->getOperand(1), op2, "", BI);
        op3 = BinaryOperator::CreateOr(op1, op2, "", BI);
        op = BinaryOperator::CreateXor(op, op3, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void substituteAnd(BinaryOperator* BI)
    {
        int choice = rand() % NUMBER_AND_SUBST;
        switch (choice) {
        case 0:
            andSubstitute(BI);
            break;
        case 1:
            andSubstituteRand(BI);
            break;
        default:
            break;
        }
    }

    void andSubstitute(BinaryOperator* BI)
    {
        BinaryOperator* op;
        op = BinaryOperator::CreateNot(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateXor(BI->getOperand(0), op, "", BI);
        op = BinaryOperator::CreateAnd(op, BI->getOperand(0), "", BI);
        BI->replaceAllUsesWith(op);
    }

    void andSubstituteRand(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1;
        op = BinaryOperator::CreateNot(BI->getOperand(0), "", BI);
        op1 = BinaryOperator::CreateNot(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateOr(op, op1, "", BI);
        op = BinaryOperator::CreateNot(op, "", BI);
        op1 = BinaryOperator::CreateNot(r, "", BI);
        op1 = BinaryOperator::CreateOr(r, op1, "", BI);
        op = BinaryOperator::CreateAnd(op, op1, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void substituteOr(BinaryOperator* BI)
    {
        int choice = rand() % NUMBER_OR_SUBST;
        switch (choice) {
        case 0:
            orSubstitute(BI);
            break;
        case 1:
            orSubstituteRand(BI);
            break;
        default:
            break;
        }
    }

    void orSubstitute(BinaryOperator* BI)
    {
        BinaryOperator *op, *op1;
        op = BinaryOperator::CreateAnd(BI->getOperand(0), BI->getOperand(1), "", BI);
        op1 = BinaryOperator::CreateXor(BI->getOperand(0), BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateOr(op, op1, "", BI);
        BI->replaceAllUsesWith(op);
    }

    void orSubstituteRand(BinaryOperator* BI)
    {
        ConstantInt* r = (ConstantInt*)ConstantInt::get(BI->getType(), rand());
        BinaryOperator *op, *op1;
        op = BinaryOperator::CreateNot(BI->getOperand(0), "", BI);
        op1 = BinaryOperator::CreateNot(BI->getOperand(1), "", BI);
        op = BinaryOperator::CreateAnd(op, op1, "", BI);
        op = BinaryOperator::CreateNot(op, "", BI);
        op1 = BinaryOperator::CreateNot(r, "", BI);
        op1 = BinaryOperator::CreateOr(r, op1, "", BI);
        op = BinaryOperator::CreateAnd(op, op1, "", BI);
        BI->replaceAllUsesWith(op);
    }
}
}

class Substitution : public FunctionPass {
public:
    static char ID;
    Substitution()
        : FunctionPass(ID)
    {
        srand(time(NULL));
    }

    bool runOnFunction(Function& F);
};

PreservedAnalyses llvm::Substitution::run(Module& M, ModuleAnalysisManager& AM)
{
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    bool is_processed = false;
    if (substitution.model) {
        for (llvm::Function& F : M) {
            if (shouldSkip(F, substitution)) {
                continue;
            }
            for (int i = 0; i < sub_times; i++) {
                for (BasicBlock& BB : F) {
                    vector<Instruction*> origInst;
                    for (Instruction& I : BB) {
                        origInst.push_back(&I);
                    }
                    for (Instruction* I : origInst) {
                        if (isa<BinaryOperator>(I)) {
                            BinaryOperator* BI = cast<BinaryOperator>(I);
                            Generic_obfuscator::Substitution::substitute(BI);
                        }
                    }
                }
            }
            PrintSuccess("Substitution successfully process func ", F.getName().str());
            is_processed = true;
        }
    }
    if (is_processed) {
        return PreservedAnalyses::none();
    } else {
        return PreservedAnalyses::all();
    }
}