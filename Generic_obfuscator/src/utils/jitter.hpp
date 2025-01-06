#ifndef OMVLL_JITTER_H
#define OMVLL_JITTER_H
#include <string>
#include <memory>

#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>

namespace llvm {
class LLVMContext;
class Module;
class MemoryBuffer;
namespace orc {
class LLJIT;
}

namespace object {
class ObjectFile;
}
}

namespace Kotoamatsukami {
class Jitter {
public:
  Jitter(const std::string &Triple);

  std::unique_ptr<llvm::orc::LLJIT> compile(llvm::Module& M);

  std::unique_ptr<llvm::MemoryBuffer> jitAsm(const std::string& Asm, size_t Size);

  std::unique_ptr<llvm::Module> compileCToIR(const std::string &CCode);
private:
  std::string Triple_;
  std::unique_ptr<llvm::LLVMContext> Ctx_;
};
}
#endif
