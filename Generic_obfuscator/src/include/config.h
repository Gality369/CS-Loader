#ifndef Kotoamatsukami_CONFIG_H
#define Kotoamatsukami_CONFIG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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
extern FunctionSettings ForObs;
extern FunctionSettings SplitBasicBlocks;
extern FunctionSettings branch2call;
extern FunctionSettings branch2call_32;
extern FunctionSettings Junkcode;
extern FunctionSettings Antihook;
extern FunctionSettings Antidebug;
extern FunctionSettings indirect_branch;
extern FunctionSettings indirect_call;
extern FunctionSettings bogus_control_flow;
extern FunctionSettings substitution;
extern FunctionSettings flatten;
extern FunctionSettings gv_encrypt;
extern Arch targetArch ;
extern int isConfigured;
extern int x[2048];
Arch parseArch(const std::string& target);
void parseConfig(const std::string& filename);
void readConfig(const std::string& filename);
std::string archToString(Arch arch);
#endif  // CONFIG_H


