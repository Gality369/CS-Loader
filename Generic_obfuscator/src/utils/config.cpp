#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "json.hpp"
#include "config.h"

FunctionSettings loopen;
FunctionSettings ForObs;
FunctionSettings SplitBasicBlocks;
FunctionSettings branch2call;
FunctionSettings branch2call_32;
FunctionSettings Junkcode;
FunctionSettings Antihook;
FunctionSettings Antidebug;
FunctionSettings indirect_branch;
FunctionSettings indirect_call;
FunctionSettings bogus_control_flow;
FunctionSettings substitution;
FunctionSettings flatten;
FunctionSettings gv_encrypt;
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
void parseConfig(const std::string& filename) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "Unable to open config file!" << std::endl;
        return;
    }
    json config;
    configFile >> config;
    targetArch = parseArch(config["target"]);
    auto parseFunctionSettings = [](const json& j, FunctionSettings& settings) {
        settings.model = j["model"];
        settings.enable_function = j["enable function"].get<std::vector<std::string>>();
        settings.disable_function = j["disable function"].get<std::vector<std::string>>();
        if (j.contains("split number")) {
            settings.op1 = j["split number"];
        }
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
        
    };

    parseFunctionSettings(config["loopen"], loopen);
    parseFunctionSettings(config["ForObs"], ForObs);
    parseFunctionSettings(config["SplitBasicBlocks"], SplitBasicBlocks);
    parseFunctionSettings(config["branch2call"], branch2call);
    parseFunctionSettings(config["branch2call_32"], branch2call_32);
    parseFunctionSettings(config["Junkcode"], Junkcode);
    parseFunctionSettings(config["Antihook"], Antihook);
    parseFunctionSettings(config["Antidebug"], Antidebug);
    parseFunctionSettings(config["indirect_branch"], indirect_branch);
    parseFunctionSettings(config["indirect_call"], indirect_call);
    parseFunctionSettings(config["bogus_control_flow"], bogus_control_flow);
    parseFunctionSettings(config["substitution"], substitution);
    parseFunctionSettings(config["flatten"], flatten);
    parseFunctionSettings(config["gv_encrypt"], gv_encrypt);
    
}

// 读取配置文件的函数
void readConfig(const std::string& filename){
    std::string fileName = "/tmp/KObfucator/KObfucator.config"; // 当前目录下的配置文件
    if (!isConfigured) {
        parseConfig(filename); // 如果未读取，解析配置
        isConfigured = true;     // 标记为已读取
    } else {
        std::cout << "Configuration already read." << std::endl; // 提示已读取
    }
}



