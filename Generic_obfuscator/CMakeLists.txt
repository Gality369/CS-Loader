cmake_minimum_required(VERSION 3.10)
project(KObfucator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Find LLVM package
set(LLVM_DIR "/home/zzzccc/llvm-17/llvm-project/build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
# Print LLVM information
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 引入json处理库
# 添加 nlohmann/json 作为子模块
include(FetchContent)

FetchContent_Declare(
    json
    SOURCE_DIR /home/zzzccc/cxzz/KObfucator/lib/json
)

FetchContent_MakeAvailable(json)

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories("/home/zzzccc/cxzz/KObfucator/src/include")
include_directories("/home/zzzccc/cxzz/KObfucator/lib/json/include")
include_directories("/home/zzzccc/cxzz/KObfucator/lib/json/single_include/nlohmann")

link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "WORKING_DIRECTORY: ${WORKING_DIRECTORY}")
include(AddLLVM)
include(HandleLLVMOptions)

add_definitions("${LLVM_DEFINITIONS}")

add_llvm_pass_plugin(KObfucator 
    src/PassPlugin.cpp
    src/pass/AddJunkCodePass.cpp
    src/pass/Branch2Call.cpp
    src/pass/Branch2Call_32.cpp
    src/pass/ForObsPass.cpp
    src/pass/Loopen.cpp
    src/pass/AntiDebugPass.cpp
    src/pass/SplitBasicBlock.cpp
    src/pass/IndirectBranch.cpp
    src/pass/IndirectCall.cpp
    src/pass/BogusControlFlow.cpp
    src/pass/Substitution.cpp
    src/pass/Flatten.cpp
    src/pass/GVEncrypt.cpp
    src/utils/utils.cpp
    src/utils/config.cpp
    src/utils/TaintAnalysis.cpp
    )
# # # Define the executable
# add_executable(jit_asm 
#               src/utils/jitter.cpp
#               src/utils/json_utils.cpp
#               src/utils/o-mvll_utils.cpp)

# Link against LLVM libraries
# 链接LLVM库
# llvm_map_components_to_libnames(llvm_libs 
#   ExecutionEngine
#   MCJIT
#   OrcJIT
#   Core
#   Support
#   nativecodegen
#   X86AsmParser
#   X86CodeGen
#   X86Desc
#   X86Disassembler
#   X86Info
#   native
#   irreader 
# )

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
# Use LLVM's target link libraries
# target_link_libraries(KObfucator ${llvm_libs} ${LLVM_LIBRARIES})

target_link_libraries(KObfucator nlohmann_json::nlohmann_json)
