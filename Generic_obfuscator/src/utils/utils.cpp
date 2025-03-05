#include "utils.hpp"
#include "llvm/IR/BasicBlock.h"


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
    // llvm::outs() << "[utils]: start createFuncFromGenerated " << moduleName << " " << funcName << "\n";
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
    // llvm::outs() << "[utils]: Function " << funcName << " successfully cloned into the target module.\n";

    return NewF;
}

Function* createFuncFromString(Module* M, std::string funcName, std::string irString)
{
    LLVMContext& Context = M->getContext();
    
    // 创建MemoryBuffer从字符串输入
    std::unique_ptr<MemoryBuffer> buffer = MemoryBuffer::getMemBuffer(irString, "IRFromString");

    // 创建一个 SMDiagnostic 来处理错误
    SMDiagnostic Err;

    // 解析 IR 到模块
    std::unique_ptr<Module> SrcModule = parseIR(buffer->getMemBufferRef(), Err, Context);

    // 错误处理，检查模块是否正确加载
    if (!SrcModule) {
        Err.print("createFuncFromString", errs());
        return nullptr;
    }

    // 在加载的模块中查找指定的函数
    Function* SrcFunc = SrcModule->getFunction(funcName);
    if (!SrcFunc) {
        errs() << "Function " << funcName << " not found in the input string.\n";
        return nullptr;
    }

    // 创建新的函数
    auto* NewF = Function::Create(SrcFunc->getFunctionType(), GlobalValue::PrivateLinkage, 
                                   funcName, M);

    // 参数映射
    ValueToValueMapTy VMap;
    auto NewFArgsIt = NewF->arg_begin();
    auto FArgsIt = SrcFunc->arg_begin();

    // 将参数映射到新函数
    for (auto FArgsEnd = SrcFunc->arg_end(); FArgsIt != FArgsEnd; ++NewFArgsIt, ++FArgsIt) {
        VMap[&*FArgsIt] = &*NewFArgsIt;
    }

    // 克隆函数内容
    SmallVector<ReturnInst*, 8> Returns;
    CloneFunctionInto(NewF, SrcFunc, VMap, CloneFunctionChangeType::DifferentModule, Returns);

    // 保留函数属性
    NewF->setCallingConv(SrcFunc->getCallingConv());
    NewF->setAttributes(SrcFunc->getAttributes());
    NewF->setDSOLocal(true);
    
    // llvm::outs() << "[utils]: Function " << funcName << " successfully cloned into the target module.\n";
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

// generate a random vector with elements not repeated
std::vector<int> generateUniqueRandomNumbers(int min, int max, int size) {
    std::unordered_set<int> uniqueNumbers;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);

    while (uniqueNumbers.size() < size) {
        uniqueNumbers.insert(dist(gen));
    }

    return std::vector<int>(uniqueNumbers.begin(), uniqueNumbers.end());
}

void fixStack(Function &F) {
  // Insert all new allocas into entry block.
  BasicBlock *BBEntry = &F.getEntryBlock();
  assert(pred_empty(BBEntry) &&
         "Entry block to function must not have predecessors!");

  // Find first non-alloca instruction and create insertion point. This is
  // safe if block is well-formed: it always have terminator, otherwise
  // we'll get and assertion.
  BasicBlock::iterator I = BBEntry->begin();
  while (isa<AllocaInst>(I))
    ++I;

  CastInst *AllocaInsertionPoint = new BitCastInst(
      Constant::getNullValue(Type::getInt32Ty(F.getContext())),
      Type::getInt32Ty(F.getContext()), "fix_stack_point", &*I);

  // Find the escaped instructions. But don't create stack slots for
  // allocas in entry block.
  std::list<Instruction *> WorkList;
  for (BasicBlock &BB : F)
    for (Instruction &I : BB)
      if (!(isa<AllocaInst>(I) && I.getParent() == BBEntry) && valueEscapes(I))
        WorkList.push_front(&I);

  // Demote escaped instructions
  //NumRegsDemoted += WorkList.size();
  for (Instruction *I : WorkList)
    DemoteRegToStack(*I, false, AllocaInsertionPoint);

  WorkList.clear();

  // Find all phi's
  for (BasicBlock &BB : F)
    for (auto &Phi : BB.phis())
      WorkList.push_front(&Phi);

  // Demote phi nodes
  //NumPhisDemoted += WorkList.size();
  for (Instruction *I : WorkList)
    DemotePHIToStack(cast<PHINode>(I), AllocaInsertionPoint);
}


bool containsPHI(BasicBlock *BB) {
    for (Instruction &I : *BB) {
        if (isa<PHINode>(&I)) {
            return true;
        }
    }
    return false;
}

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <set>
/// @brief 将一个基本块随机拆分为指定数量的块。
///
/// @param BB 要拆分的基本块。
/// @param numBlocks 要拆分成的块的数量（必须大于 1）。
///
/// @return 一个包含新创建的基本块的向量（包括原始基本块）。
///         向量中的基本块按照它们在控制流中的顺序排列。
std::vector<BasicBlock*> splitBasicBlockRandomly(BasicBlock *BB, int numBlocks) {
    if (numBlocks < 3){
        return {BB};
    }
    // 参数验证
    if (!BB || BB->size() < 2 || containsPHI(BB)) {
        // llvm::errs() << "Cannot split basic block: " << BB->getName() << "\n";
        return {};
    }

    int splitCount = numBlocks - 1;
    if (splitCount <= 0) {
        // llvm::outs() << "No split needed for basic block: " << BB->getName() << "\n";
        return {BB};
    }

    int maxPossibleSplits = BB->size() - 1;
    splitCount = std::min(splitCount, maxPossibleSplits);
    if (splitCount == 0) {
        // llvm::outs() << "Split count adjusted to 0 for basic block: " << BB->getName() << "\n";
        return {BB};
    }

    // 生成唯一的分割点
    std::set<int> splitPoints;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, maxPossibleSplits);

    while (splitPoints.size() < splitCount) {
        int pt = distrib(gen);
        splitPoints.insert(pt);
    }

    std::vector<int> sortedSplitPoints(splitPoints.begin(), splitPoints.end());
    std::sort(sortedSplitPoints.begin(), sortedSplitPoints.end());

    // llvm::outs() << "Splitting basic block: " << BB->getName() 
                //  << " into " << numBlocks << " blocks at points: ";
    // for (int pt : sortedSplitPoints) {
        // llvm::outs() << pt << " ";
    // }
    // llvm::outs() << "\n";

    std::vector<BasicBlock*> result;
    result.push_back(BB); // 初始包含原块

    BasicBlock *currentBB = BB;
    int currentStartOffset = 0;

    for (int pt : sortedSplitPoints) {
        // 验证分割点有效性
        if (pt < currentStartOffset || pt >= currentStartOffset + currentBB->size()) {
            // llvm::errs() << "Invalid split point " << pt << " in block " << BB->getName() << "\n";
            return {};
        }

        int relativePt = pt - currentStartOffset;
        BasicBlock::iterator it = currentBB->begin();
        std::advance(it, relativePt);

        // 执行分割并收集新块
        BasicBlock *newBB = currentBB->splitBasicBlock(it, currentBB->getName() + ".split");
        result.push_back(newBB);
        currentBB = newBB;
        currentStartOffset = pt + 1;
    }

    return result;
}

#include <iostream>
#include <string>
#include <cstdarg>
void PrintZZZCCC(){
    std::cout << "\033[36m";
    std::cout << " ________  ________  ________  ________  ________  ________     " << std::endl;
    std::cout << "|\\_____  \\|\\_____  \\|\\_____  \\|\\   ____\\|\\   ____\\|\\   ____\\    " << std::endl;
    std::cout << " \\|___/  /|\\|___/  /|\\|___/  /\\ \\  \\___|\\ \\  \\___|\\ \\  \\___|    " << std::endl;
    std::cout << "     /  / /    /  / /    /  / /\\ \\  \\    \\ \\  \\    \\ \\  \\       " << std::endl;
    std::cout << "    /  /_/__  /  /_/__  /  /_/__\\ \\  \\____\\ \\  \\____\\ \\  \\____  " << std::endl;
    std::cout << "   |\\________\\\\________\\\\________\\ \\_______\\ \\_______\\ \\_______\\" << std::endl;
    std::cout << "    \\|_______|\\|_______|\\|_______|\\|_______|\\|_______|\\|_______|" << std::endl;
    std::cout << "\033[0m";
}
