#include "utils.hpp"


using namespace llvm;
using std::vector;

static bool valueEscapes(const Instruction& Inst)
{
    const BasicBlock* BB = Inst.getParent();
    for (const User* U : Inst.users()) {
        const Instruction* UI = cast<Instruction>(U);
        if (UI->getParent() != BB || isa<PHINode>(UI))
            return true;
    }
    return false;
}

Function* createFuncFromGenerated(Module* M, std::string funcName, std::string moduleName)
{
    // 创建一个新的LLVM上下文
    LLVMContext& Context = M->getContext();

    // 读取模块文件并加载它
    SMDiagnostic Err;
    llvm::outs() << "[utils]: start createFuncFromGenerated " << moduleName << " " << funcName << "\n";
    Context.setDiscardValueNames(false);
    std::unique_ptr<Module> SrcModule = parseIRFile(moduleName, Err, Context);

    // 错误处理，检查模块是否正确加载
    if (!SrcModule) {
        Err.print("createFuncFromGenerated", errs());
        return nullptr;
    }
    // SrcModule->print(llvm::outs(),nullptr);
    // 在加载的模块中查找指定的函数
    Function* SrcFunc = SrcModule->getFunction(funcName);
    if (!SrcFunc) {
        errs() << "Function " << funcName << " not found in " << moduleName << "\n";
        return nullptr;
    }

    auto* NewF = Function::Create(SrcFunc->getFunctionType(), GlobalValue::PrivateLinkage,
        funcName, M);

    ValueToValueMapTy VMap;
    auto NewFArgsIt = NewF->arg_begin();
    auto FArgsIt = SrcFunc->arg_begin();

    // 将参数映射到新函数
    for (auto FArgsEnd = SrcFunc->arg_end(); FArgsIt != FArgsEnd;
         ++NewFArgsIt, ++FArgsIt) {
        VMap[&*FArgsIt] = &*NewFArgsIt;
    }

    SmallVector<ReturnInst*, 8> Returns;
    CloneFunctionInto(NewF, SrcFunc, VMap, CloneFunctionChangeType::DifferentModule, Returns);

    // 保留函数属性
    NewF->setCallingConv(SrcFunc->getCallingConv());
    NewF->setAttributes(SrcFunc->getAttributes());
    NewF->setDSOLocal(true);
    llvm::outs() << "[utils]: Function " << funcName << " successfully cloned into the target module.\n";

    return NewF;
}

uint64_t getRandomNumber()
{
    return (((uint64_t)rand()) << 32) | ((uint64_t)rand());
}

void demoteRegisters(Function* f)
{
    std::vector<PHINode*> tmpPhi;
    std::vector<Instruction*> tmpReg;
    BasicBlock* bbEntry = &*f->begin();
    for (Function::iterator i = f->begin(); i != f->end(); i++) {
        for (BasicBlock::iterator j = i->begin(); j != i->end(); j++) {
            if (isa<PHINode>(j)) {
                PHINode* phi = cast<PHINode>(j);
                tmpPhi.push_back(phi);
                continue;
            }
            if (!(isa<AllocaInst>(j) && j->getParent() == bbEntry) && j->isUsedOutsideOfBlock(&*i)) {
                tmpReg.push_back(&*j);
                continue;
            }
        }
    }
    for (unsigned int i = 0; i < tmpReg.size(); i++)
        DemoteRegToStack(*tmpReg.at(i), f->begin()->getTerminator());
    for (unsigned int i = 0; i < tmpPhi.size(); i++)
        DemotePHIToStack(tmpPhi.at(i), f->begin()->getTerminator());
}

BasicBlock* cloneBasicBlock(BasicBlock* BB)
{
    ValueToValueMapTy VMap;
    // 在克隆基本块时，如果不进行映射，克隆出来的指令会使用原始基本块中的操作数 而Vmap 中存储了Value的对应映射关系 只需要换过来即可
    BasicBlock* cloneBB = CloneBasicBlock(BB, VMap, "cloneBB", BB->getParent());
    BasicBlock::iterator origI = BB->begin();
    for (Instruction& I : *cloneBB) {
        for (int i = 0; i < I.getNumOperands(); i++) {
            Value* V = MapValue(I.getOperand(i), VMap);
            if (V) {
                I.setOperand(i, V);
            }
        }
        SmallVector<std::pair<unsigned, MDNode*>, 4> MDs;
        I.getAllMetadata(MDs);
        for (std::pair<unsigned, MDNode*> pair : MDs) {
            MDNode* MD = MapMetadata(pair.second, VMap);
            if (MD) {
                I.setMetadata(pair.first, MD);
            }
        }
        I.setDebugLoc(origI->getDebugLoc());
        origI++;
    }
    return cloneBB;
}

std::string getInstructionAsString(llvm::Instruction* I)
{
    std::string output;
    llvm::raw_string_ostream stream(output);
    I->print(stream); // 将指令输出到 stream 中
    return output; // 返回捕获的字符串
}