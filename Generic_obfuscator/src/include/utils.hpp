// 参考 https://github.com/DreamSoule/ollvm17
#ifndef KObfucator_UTILS_H
#define KObfucator_UTILS_H
// LLVM libs
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Local.h>
#include "llvm/IRReader/IRReader.h" // For parseIRFile
#include "llvm/Support/SourceMgr.h" // For SMDiagnostic

using namespace std;
using namespace llvm;
void demoteRegisters(llvm::Function *f);
llvm::Function *createFuncFromGenerated(llvm::Module *M, std::string funcName, std::string moduleName);
uint64_t getRandomNumber();
BasicBlock* cloneBasicBlock(BasicBlock* BB);
#endif // LLVM_UTILS_H