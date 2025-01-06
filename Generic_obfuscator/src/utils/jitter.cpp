#include "jitter.hpp"
#include <llvm/AsmParser/Parser.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Lex/PreprocessorOptions.h>

#include "./json_utils.hpp"
#include "o-mvll_utils.hpp"
#include <fmt/format.h>
// #include <llvm/TargetParser/Triple.h>
// #include <llvm/TargetParser/Host.h>
using namespace llvm;
using namespace llvm::orc;
using namespace omvll;
ExitOnError ExitOnErr;

namespace Kotoamatsukami
{
  // 构造函数
  Jitter::Jitter(const std::string &Triple)
      : Triple_{Triple}, Ctx_{new LLVMContext{}}
  {
    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();
  }

  ThreadSafeModule createModuleWithInlineAsm(const std::string &Asm, const std::string &FName)
  {
    auto Context = std::make_unique<LLVMContext>();
    auto M = std::make_unique<llvm::Module>("__Kotoamatsukami_asm_jit", *Context);

    // Create the function
    Function *F = Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(*Context), {}, false),
        Function::ExternalLinkage,
        FName,
        M.get());

    // Create the entry basic block
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*Context, "EntryBlock", F);
    IRBuilder<> builder(BB);

    // Define the function type for inline assembly
    auto *FType = llvm::FunctionType::get(builder.getVoidTy(), false);

    // Create the inline assembly
    InlineAsm *rawAsm = InlineAsm::get(FType, Asm, "",
                                       /* hasSideEffects */ true,
                                       /* isStackAligned */ true);

    // Create a call to the inline assembly and a return statement
    builder.CreateCall(FType, rawAsm);
    builder.CreateRetVoid();

    // Create and return the ThreadSafeModule
    return ThreadSafeModule(std::move(M), std::move(Context));
  }

  std::unique_ptr<MemoryBuffer> Jitter::jitAsm(const std::string &Asm, size_t Size)
  {
    static constexpr const char FNAME[] = "__Kotoamatsukami_asm_func";
    auto M = createModuleWithInlineAsm(Asm, FNAME);
    ExitOnErr.setBanner("Kotoamatsukami_jitasm: ");
    auto J = ExitOnErr(LLJITBuilder().create());
    ExitOnErr(J->addIRModule(std::move(M)));
    auto Func = J->lookup(FNAME);
    size_t FunSize = Size * /* Sizeof AArch64 inst=*/4;
    uint64_t Addr = Func->getValue();
    auto *ptr = reinterpret_cast<const char *>(Addr);
    return MemoryBuffer::getMemBufferCopy({ptr, FunSize});
  }
  void compilerCToIR(const Triple &TargetTriple, llvm::Module *TM, llvm::Module *HM, LLVMContext &Ctx, const std::string &CCode)
  {
    // 包装输入的 C 代码
    std::string WrappedCode = (Twine("extern \"C\" {\n") + CCode + "\n}\n").str();

    // 使用 ExitOnError 处理错误
    ExitOnError ExitOnErr("Failed to compile C code: ");

    // 为目标平台生成模块，并应用注解和优化
    {
      // 禁用丢弃值名称以便调试
      Ctx.setDiscardValueNames(false);

      // 生成目标平台的模块
      TM = ExitOnErr(
          generateModule(WrappedCode, TargetTriple, "cpp", Ctx,
                         {"-target", TargetTriple.getTriple(), "-std=c++17",
                          "-mllvm", "--opaque-pointers"}));
    }
  }

}

// int main(int argc, char *argv[])
// {
//   // Initialize LLVM components
//   LLVMInitializeNativeTarget();
//   // Load the LLVM IR file
//   LLVMContext Context;
//   SMDiagnostic Error;
//   std::unique_ptr<Module> M = parseIRFile("/home/zzzccc/cxzz/Kotoamatsukami/test/test1.ll", Error, Context);
//   // Create a Jitter instance using the current platform's triple
//   std::string TargetTriple = M->getTargetTriple();
//   if (TargetTriple.empty()) {
//       TargetTriple = llvm::sys::getDefaultTargetTriple();
//   }
//   outs() << TargetTriple;
//   Kotoamatsukami::Jitter jitter(TargetTriple);

//   if (!M)
//   {
//     Error.print(argv[0], errs());
//     return 1;
//   }
//   // Function name in the LLVM IR file where you want to inject the assembly code
//   std::string functionName = "main";

//   // Assembly code to JIT compile and inject into the function's preamble
//   std::string asmCode = "movq 60, %rax; xor %rdi, %rdi; syscall";
//   // Compile the assembly code and get a buffer containing the compiled machine code
//   size_t codeSize = 3; // Adjust based on the number of assembly instructions and platform
//   auto insts = jitter.jitAsm(asmCode, codeSize);
//   llvm::Function *F = M->getFunction(functionName);
//   if (!F)
//   {
//     errs() << "Function not found: " << functionName << "\n";
//     return 1;
//   }
//   auto *Int8Ty = Type::getInt8Ty(F->getContext());
//   auto *Prologue = llvm::ConstantDataVector::get(Context, llvm::ArrayRef<uint8_t>((const uint8_t *)insts->getBuffer().bytes_begin(), insts->getBuffer().size()));
//   F->setPrologueData(Prologue);
//   llvm::outs() << "Modified LLVM IR:\n";
//   M->print(llvm::outs(), nullptr);
//   return 0;
// }

// 使用示例
int main(int argc, char *argv[])
{
  LLVMInitializeNativeTarget();
  LLVMContext Context;

  // 创建Jitter实例
  Kotoamatsukami::Jitter jitter(llvm::sys::getDefaultTargetTriple());

  // C语言代码字符串
  std::string CCode = R"(
    int add(int a, int b) {
        return a + b;
    }
  )";
  std::vector<JsonUtils::QuadraticFunction> QuadraticFunVec = JsonUtils::getForPassFunc("/home/zzzccc/cxzz/Kotoamatsukami/config/quadratic_equations.json");
  llvm::Module TM = nullptr;
  llvm::Module HM = nullptr;
  // 将C代码编译为LLVM IR
  jitter.compileCToIR(llvm::sys::getDefaultTargetTriple(), TM, HM, Context, QuadraticFunVec[0].cFunc);

  if (TM)
  {
    // 打印生成的IR
    TM->print(llvm::outs(), nullptr);
  }

  return 0;
}