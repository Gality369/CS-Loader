#include "AntiDebugPass.h"
#include "config.h"
#include "utils.hpp"
using namespace llvm;
std::vector<llvm::Function*> defineAntiDebugFunc(llvm::Module *M, LLVMContext &Context) {
    // 创建一个存储函数的vector
    std::vector<llvm::Function*> functions;
    // 可以继续创建更多函数
    for (int i = 1; i <= 7; ++i) {
        std::string funcName = "KObfucator_Antidebug" + std::to_string(i);
        FunctionType *FT = FunctionType::get(Type::getVoidTy(Context), false);
        Function *F = Function::Create(FT, Function::ExternalLinkage, funcName, M);
        functions.push_back(F);
    }
    // 返回包含所有函数的vector
    return functions;
}

// 插入构造函数，将7个反调试函数加入到llvm.global_ctors
void insertConstructorFunctions(llvm::Module &module, std::vector<llvm::Function*> &antidebugFuncs) {
    llvm::LLVMContext &context = module.getContext();
    
    // 创建结构体类型，存储（优先级, 函数指针）
    llvm::StructType *ctorStructType = llvm::StructType::get(
        context, {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)}
    );
    
    // 创建全局的 llvm.global_ctors 数组，用来存储构造函数
    llvm::ArrayType *globalCtorType = llvm::ArrayType::get(ctorStructType, antidebugFuncs.size());
    llvm::GlobalVariable *globalCtor = new llvm::GlobalVariable(
        module, globalCtorType, false, llvm::GlobalValue::AppendingLinkage, nullptr, "llvm.global_ctors"
    );

    // 创建每个构造函数条目的常量
    std::vector<llvm::Constant*> elements;
    for (auto &antidebugFunc : antidebugFuncs) {
        llvm::Constant *ctorStruct = llvm::ConstantStruct::get(
            ctorStructType, {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 65535), // 优先级，65535是默认值
                llvm::ConstantExpr::getBitCast(antidebugFunc, llvm::Type::getInt8PtrTy(context)) // 函数指针
            }
        );
        elements.push_back(ctorStruct);
    }

    // 将所有条目加入到全局变量 llvm.global_ctors 中
    globalCtor->setInitializer(llvm::ConstantArray::get(globalCtorType, elements));
}

PreservedAnalyses AntiDebugPass::run(Module &M, ModuleAnalysisManager &AM)
{
    bool isChanged = false;
    readConfig("/home/zzzccc/cxzz/KObfucator/config/config.json");
    if (Antidebug.model)
    {
            std::vector<llvm::Function*> antiDebugFuncs = defineAntiDebugFunc(&M, M.getContext());

            // 插入构造函数
            insertConstructorFunctions(M, antiDebugFuncs);

            isChanged = true;
    }

    if (isChanged)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}