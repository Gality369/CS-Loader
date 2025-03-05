#include "AntiDebugPass.h"
#include "config.h"
#include "utils.hpp"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <vector>
#include "Log.hpp"
using namespace llvm;
/*
function prototype
void Generic_obfuscator_Antidebug1() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        printf("Debugger detected!\n");
        _exit(1);
    }
    printf("No debugger detected, continuing...\n");
}
*/
llvm::Function* createAntiDebugFunc1(llvm::Module* M)
{
    auto& context = M->getContext();
    FunctionType* funcType = FunctionType::get(Type::getVoidTy(context), false);
    Function* antiDebugFunc = Function::Create(funcType, Function::ExternalLinkage, "Generic_obfuscator_Antidebug1", M);
    if (!antiDebugFunc) {
        PrintError("Error creating AntiDebugFUNC1");
        return nullptr;
    }
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", antiDebugFunc);
    IRBuilder<> builder(entryBlock);

    FunctionType* ptraceType = FunctionType::get(Type::getInt32Ty(context), { Type::getInt32Ty(context), Type::getInt64Ty(context), Type::getInt64Ty(context), Type::getInt64Ty(context) }, false);
    FunctionCallee ptraceFunc = M->getOrInsertFunction("ptrace", ptraceType);

    FunctionType* printfType = FunctionType::get(Type::getInt32Ty(context), { Type::getInt8PtrTy(context) }, true);
    FunctionCallee printfFunc = M->getOrInsertFunction("printf", printfType);

    FunctionType* exitType = FunctionType::get(Type::getVoidTy(context), { Type::getInt32Ty(context) }, false);
    FunctionCallee exitFunc = M->getOrInsertFunction("_exit", exitType);


    Constant* debuggerDetectedStr = builder.CreateGlobalStringPtr("Debugger detected!\n");

    Value* ptraceCall = builder.CreateCall(ptraceFunc, { ConstantInt::get(Type::getInt32Ty(context), 0), ConstantInt::get(Type::getInt64Ty(context), 0), ConstantInt::get(Type::getInt64Ty(context), 0), ConstantInt::get(Type::getInt64Ty(context), 0) });

    Value* cmp = builder.CreateICmpSLT(ptraceCall, ConstantInt::get(Type::getInt32Ty(context), 0));

    BasicBlock* ifBlock = BasicBlock::Create(context, "if", antiDebugFunc);
    BasicBlock* elseBlock = BasicBlock::Create(context, "else", antiDebugFunc);

    builder.CreateCondBr(cmp, ifBlock, elseBlock);

    // if block
    builder.SetInsertPoint(ifBlock);
    builder.CreateCall(printfFunc, { debuggerDetectedStr });
    builder.CreateCall(exitFunc, { ConstantInt::get(Type::getInt32Ty(context), 1) });
    builder.CreateUnreachable();

    // else block
    builder.SetInsertPoint(elseBlock);
    builder.CreateRetVoid();
    return antiDebugFunc;
}
/*
function prototype
void Generic_obfuscator_Antidebug2() {
    pid_t ppid = getppid();
    pid_t sid = getsid(getpid());

    if (sid != ppid) {
        printf("Debugger detected based on session and parent PID mismatch!\n");
        _exit(1);
    } else {
        printf("No debugger detected, continuing...\n");
    }
}
*/
Function* createAntiDebugFunc2(Module *module) {
    LLVMContext& context = module->getContext();
    FunctionType *funcType = FunctionType::get(Type::getVoidTy(context), false);
    Function *antiDebugFunc = Function::Create(funcType, Function::ExternalLinkage, "Generic_obfuscator_Antidebug2", module);
    BasicBlock *entryBlock = BasicBlock::Create(context, "entry", antiDebugFunc);
    IRBuilder<> builder(entryBlock);

     // Declare getppid, getsid, and getpid functions
    FunctionType* getpidType = FunctionType::get(Type::getInt32Ty(context), {}, false);
    FunctionCallee getpidFunc = module->getOrInsertFunction("getpid", getpidType);

    FunctionType* getppidType = FunctionType::get(Type::getInt32Ty(context), {}, false);
    FunctionCallee getppidFunc = module->getOrInsertFunction("getppid", getppidType);

     FunctionType* getsidType = FunctionType::get(Type::getInt32Ty(context), {Type::getInt32Ty(context)}, false);
     FunctionCallee getsidFunc = module->getOrInsertFunction("getsid", getsidType);


    // Declare printf function
    FunctionType* printfType = FunctionType::get(Type::getInt32Ty(context), { Type::getInt8PtrTy(context) }, true);
    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

      // Declare exit function
    FunctionType* exitType = FunctionType::get(Type::getVoidTy(context), { Type::getInt32Ty(context) }, false);
    FunctionCallee exitFunc = module->getOrInsertFunction("_exit", exitType);


    // Get string constants
    Constant *debuggerDetectedStr = builder.CreateGlobalStringPtr("Debugger detected based on session and parent PID mismatch!\n");
    Constant *noDebuggerDetectedStr = builder.CreateGlobalStringPtr("No debugger detected, continuing...\n");

    // Call getppid and getpid
    Value* ppid = builder.CreateCall(getppidFunc, {});
    Value* pid = builder.CreateCall(getpidFunc, {});

    // Call getsid
    Value* sid = builder.CreateCall(getsidFunc, {pid});

    // Compare sid and ppid
    Value* cmp = builder.CreateICmpNE(sid, ppid);

    // Create if-else blocks
    BasicBlock* ifBlock = BasicBlock::Create(context, "if", antiDebugFunc);
    BasicBlock* elseBlock = BasicBlock::Create(context, "else", antiDebugFunc);
    builder.CreateCondBr(cmp, ifBlock, elseBlock);
    builder.SetInsertPoint(ifBlock);
    builder.CreateCall(printfFunc, {debuggerDetectedStr});
    builder.CreateCall(exitFunc,{ConstantInt::get(Type::getInt32Ty(context), 1)});
    builder.CreateUnreachable();
    builder.SetInsertPoint(elseBlock);
    // builder.CreateCall(printfFunc, {noDebuggerDetectedStr});
    builder.CreateRetVoid();
    return antiDebugFunc;
}

PreservedAnalyses AntiDebugPass::run(Module& M, ModuleAnalysisManager& AM)
{
    if (isInserted){
        return PreservedAnalyses::all();
    }
    bool isChanged = false;
    readConfig("/home/zzzccc/cxzz/Generic_obfuscator/config/config.json");
    if (antiDebug.model) {
        std::vector<llvm::Function*> antiDebugFuncs;
        antiDebugFuncs.push_back(createAntiDebugFunc1(&M));
        antiDebugFuncs.push_back(createAntiDebugFunc2(&M));
        if (antiDebugFuncs.size() > 0) {
            Function* antiDebugFunc = antiDebugFuncs[rand() % antiDebugFuncs.size()];
            if (M.size() > 0) {
                appendToGlobalCtors(M,antiDebugFunc, 55555);
            }
            isInserted = true;
            isChanged = true;
        }
        PrintSuccess("AntiDebug successfully process Module ", M.getName().str());
    }

    if (isChanged)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}