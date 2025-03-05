#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include "json.hpp"
#include "config.h"
#include "Log.hpp"

FunctionSettings loopen;
FunctionSettings forObs;
FunctionSettings splitBasicBlocks;
FunctionSettings branch2call;
FunctionSettings branch2call_32;
FunctionSettings junkCode;
FunctionSettings antiHook;
FunctionSettings antiDebug;
FunctionSettings indirectBranch;
FunctionSettings indirectCall;
FunctionSettings bogusControlFlow;
FunctionSettings substitution;
FunctionSettings flatten;
FunctionSettings gvEncrypt;
Arch targetArch;
std::string target;
int isConfigured = false;
int x[2048] = {0};
using json = json;

std::string archToString(Arch arch) {
    switch (arch) {
        case Arch::UNKNOWN: return "UNKNOWN";
        case Arch::X86: return "X86";
        case Arch::X86_64: return "X86_64";
        case Arch::ARM32: return "ARM32";
        case Arch::ARM64: return "ARM64";
        case Arch::MIPS: return "MIPS";
        case Arch::POWERPC: return "POWERPC";
        case Arch::RISCV: return "RISCV";
        default: return "INVALID";
    }
}

Arch parseArch(const std::string& target) {
    // 将输入字符串转为小写以进行不区分大小写比较
    std::string lower_target = target;
    std::transform(lower_target.begin(), lower_target.end(), lower_target.begin(), ::tolower);
    if (lower_target == "x86") {
        return Arch::X86;
    } else if (lower_target == "x86_64") {
        return Arch::X86_64;
    } else if (lower_target == "arm32") {
        return Arch::ARM32;
    } else if (lower_target == "arm64") {
        return Arch::ARM64;
    } else if (lower_target == "mips") {
        return Arch::MIPS;
    } else if (lower_target == "powerpc") {
        return Arch::POWERPC;
    } else if (lower_target == "riscv") {
        return Arch::RISCV;
    } else {
        return Arch::UNKNOWN; // 返回 UNKNOWN
    }
}

// 解析 JSON 并赋值
int parseConfig(const std::string& filename) {
    PrintInfo("start to parse config file: ",filename);
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        return 0;
    }
    json config;
    configFile >> config;
    targetArch = parseArch(config["target"]);
    auto parseFunctionSettings = [](const json& j, FunctionSettings& settings) {
        settings.model = j["model"];
        settings.enable_function = j["enable function"].get<std::vector<std::string>>();
        settings.disable_function = j["disable function"].get<std::vector<std::string>>();
        // splitBasicBlocks settings
        if (j.contains("split number")) {
            settings.op1 = j["split number"];
        }
        // forObs settings
        if (j.contains("innerLoopBoundary")) {
            settings.op1 = j["innerLoopBoundary"];
        }
        if (j.contains("outerLoopBoundary")) {
            settings.op2 = j["outerLoopBoundary"];
        } 
        // Loopen settings
        if (j.contains("loopen_x_list"))
        {
            std::vector<int> tmp = j["loopen_x_list"].get<std::vector<int>>();
            for (size_t i = 0; i < std::min(tmp.size(), size_t(2048)); ++i) {
                x[i] = tmp[i];
            }
        }
        if (j.contains("module_name"))
        {
            std::string tmp = j["module_name"].get<std::string>();
            settings.module_name = tmp;
        }
        // Loopen settings end
    };

    parseFunctionSettings(config["loopen"], loopen);
    // std::cout << "Parse config: " << "loopen" << "\n";
    parseFunctionSettings(config["forObs"], forObs);
    // std::cout << "Parse config: " << "forObs" << "\n";
    parseFunctionSettings(config["splitBasicBlocks"], splitBasicBlocks);
    // std::cout << "Parse config: " << "splitBasicBlocks" << "\n";
    parseFunctionSettings(config["branch2call"], branch2call);
    // std::cout << "Parse config: " << "branch2call" << "\n";
    parseFunctionSettings(config["branch2call_32"], branch2call_32);
    // std::cout << "Parse config: " << "branch2call_32" << "\n";
    parseFunctionSettings(config["junkCode"], junkCode);
    // std::cout << "Parse config: " << "junkCode" << "\n";
    // std::cout << "Parse config: " << "antiHook" << "\n";
    // parseFunctionSettings(config["antiHook"], antiHook);
    parseFunctionSettings(config["antiDebug"], antiDebug);
    // std::cout << "Parse config: " << "antiDebug" << "\n";
    parseFunctionSettings(config["indirectBranch"], indirectBranch);
    // std::cout << "Parse config: " << "indirectBranch" << "\n";
    parseFunctionSettings(config["indirectCall"], indirectCall);
    // std::cout << "Parse config: " << "indirectCall" << "\n";
    parseFunctionSettings(config["bogusControlFlow"], bogusControlFlow);
    // std::cout << "Parse config: " << "bogusControlFlow" << "\n";
    parseFunctionSettings(config["substitution"], substitution);
    // std::cout << "Parse config: " << "substitution" << "\n";
    parseFunctionSettings(config["flatten"], flatten);
    // std::cout << "Parse config: " << "flatten" << "\n";
    parseFunctionSettings(config["gvEncrypt"], gvEncrypt);
    // std::cout << "Parse config: " << "gvEncrypt" << "\n";
    return 1;
}

void readConfig(const std::string& fileName) {
    if (!isConfigured) {
        if (parseConfig(fileName)){
            PrintSuccess("Parse Config in ",fileName);
            isConfigured = true;
        }else {
            std::filesystem::path currentDir = std::filesystem::current_path();
            std::filesystem::path filePath = currentDir / "Generic_obfuscator.config";
            if(parseConfig(filePath.string())){
                PrintSuccess("Parse Config in ",filePath.string());
                PrintZZZCCC();
                isConfigured = true;
            }
            else{
                PrintError("Fail to read the config file in ",fileName);
                exit(0);
            }
        }
    }
}

bool shouldSkip(const llvm::Function& F,const FunctionSettings& setting)
{
    std::string functionName = F.getName().str();
    if (F.size() == 1) {
        return 1;
    }
    if (functionName.find("Generic_obfuscator") != std::string::npos || functionName.find("generic_obfuscator") != std::string::npos) {
        return true;
    }
    if (F.empty() || F.hasLinkOnceLinkage() || F.getSection() == ".text.startup" || !F.hasExactDefinition()) {
        return 1;
    }

    if (setting.model == 2) {
        if (std::find(setting.enable_function.begin(),
                setting.enable_function.end(),
                functionName)
            == setting.enable_function.end()) {
            return 1;
        }
    } else if (setting.model == 3) {
        if (std::find(setting.disable_function.begin(),
                setting.disable_function.end(),
                functionName)
            != setting.disable_function.end()) {
            return 1;
        }
    }
    return 0;
}


