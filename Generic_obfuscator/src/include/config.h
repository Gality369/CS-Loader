#ifndef Generic_obfuscator_CONFIG_H
#define Generic_obfuscator_CONFIG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"
#include "json.hpp"
using json = nlohmann::json;

enum class Arch {
    UNKNOWN,       // 未知架构
    X86,       // 通用架构
    X86_64,        // 64位 x86 架构
    ARM32,         // 32位 ARM 架构
    ARM64,         // 64位 ARM 架构
    MIPS,          // MIPS 架构
    POWERPC,       // PowerPC 架构
    RISCV,         // RISC-V 架构
};

enum class ProcessModel {
    DISABLE,         // 不启用
    ENABLE,          // 全部启用
    PartiallyEnable, // 部分函数启用
    PartiallyDisable,// 部分函数不启用

};

struct FunctionSettings {
    int model;
    std::vector<std::string> enable_function;
    std::vector<std::string> disable_function;
    int op1 = 0;  
    int op2 = 0;
    std::string module_name;
};

extern FunctionSettings loopen;
extern FunctionSettings forObs;
extern FunctionSettings splitBasicBlocks;
extern FunctionSettings branch2call;
extern FunctionSettings branch2call_32;
extern FunctionSettings junkCode;
extern FunctionSettings antiHook;
extern FunctionSettings antiDebug;
extern FunctionSettings indirectCall;
extern FunctionSettings indirectBranch;
extern FunctionSettings bogusControlFlow;
extern FunctionSettings substitution;
extern FunctionSettings flatten;
extern FunctionSettings gvEncrypt;

extern Arch targetArch ;
extern int isConfigured;
extern int x[2048];
Arch parseArch(const std::string& target);
int parseConfig(const std::string& filename);
// First look for the parameters passed in as a config filed,then search the Generic_obfuscator.config in current dir
void readConfig(const std::string& filename);
std::string archToString(Arch arch);

bool shouldSkip(const llvm::Function& F,const FunctionSettings& setting);
#endif  // CONFIG_H


