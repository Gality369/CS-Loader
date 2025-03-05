#!/bin/bash
# arguments
# set -eux

Generic_obfuscator_so=/home/zzzccc/cxzz/Generic_obfuscator/build/Generic_obfuscator.so
CLANG=clang-17
OPT=opt-17
current_dir=$(pwd)
BRANCH2CALL_PROCESS="$current_dir/branch2call_process.py"
ANTIDEBUG_SOURCEFILE="$current_dir/Generic_obfuscator_Antidebug.c"
obfuscate_args=()
source_files=""
output_file=""
branch2call_enable=false
generic_obfuscator_args=()  # 用于存储 generic_obfuscator 的参数
in_generic_obfuscator_args=false # 标志是否正在读取 generic_obfuscator 的参数
in_output_arg=false
old_args=""
clang_args=""

# $CLANG $ANTIDEBUG_SOURCEFILE -O0 -emit-llvm -S -o ${ANTIDEBUG_SOURCEFILE%.c}.ll
for arg in "$@"; do
    old_args+=("$arg")
    if [[ "$arg" == "split-basic-block" ]]; then
        echo "识别到 split-basic-block"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "anti-debug" ]]; then
        echo "识别到 anti-debug"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "gv-encrypt" ]]; then
        echo "识别到 gv-encrypt"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "bogus-control-flow" ]]; then
        echo "识别到 bogus-control-flow"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "add-junk-code" ]]; then
        echo "识别到 add-junk-code"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "loopen" ]]; then
        echo "识别到 loopen"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "for-obs" ]]; then
        echo "识别到 for-obs"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "branch2call-32" ]]; then
        echo "识别到 branch2call-32"
        branch2call_enable=true
    elif [[ "$arg" == "branch2call" ]]; then
        echo "识别到 branch2call"
        generic_obfuscator_args+=("$arg")
        branch2call_enable=true
    elif [[ "$arg" == "indirect-call" ]]; then
        echo "识别到 indirect-call"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "indirect-branch" ]]; then
        echo "识别到 indirect-branch"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "flatten" ]]; then
        echo "识别到 flatten"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == "substitution" ]]; then
        echo "识别到 substitution"
        generic_obfuscator_args+=("$arg")
    elif [[ "$arg" == *".c" ]]; then
        source_files="$arg"
    elif [[ "$arg" == '-o' ]]; then
        in_output_arg=true
    elif [[ "$in_output_arg" == true ]]; then
        output_file="$arg"
        in_output_arg=false
    else
      clang_args+=("$arg")
      continue
    fi
done

# 检查是否找到了 generic_obfuscator 及其参数
if [[ -z "$source_files" || (${#generic_obfuscator_args[@]} -eq 0 && -z branch2call_enable) ]]; then
    $CLANG "$@"
    exit
fi

echo "Source file: $source_files"
echo "generic_obfuscator args: ${generic_obfuscator_args[@]}"

# 如果源文件存在且是 .c 文件，获取源文件所在的目录
if [[ -f "$source_files" && "$source_files" == *".c" ]]; then
    $CLANG -S -emit-llvm "${clang_args[@]}" $source_files -o "${source_files%.c}.ll"
    ll_file="${source_files%.c}.ll"

    passes_str=$(printf "%s," "${generic_obfuscator_args[@]}")
    passes_str=${passes_str%,}
    echo $OPT --load-pass-plugin=$Generic_obfuscator_so $ll_file --passes="$passes_str" -S -o "${ll_file%.ll}.obfuscated.ll" 
    $OPT --load-pass-plugin=$Generic_obfuscator_so $ll_file --passes="$passes_str" -S -o "${ll_file%.ll}.obfuscated.ll" 
    if [[ "$branch2call_enable" == true ]]; then
        obfuscated_ll_file="${ll_file%.ll}.obfuscated.ll"
        asm_file="${ll_file%.ll}.s"
        $CLANG "$obfuscated_ll_file" "${clang_args[@]}"  -Wno-unused-command-line-argument -S -o $asm_file
        python3 $BRANCH2CALL_PROCESS $asm_file $asm_file
        $CLANG "$asm_file" "${clang_args[@]}"  -Wno-unused-command-line-argument -o "$output_file"
        if [[ -z "$DEBUG" || "$DEBUG" != "1" ]]; then
            rm "$ll_file" "$obfuscated_ll_file" "$asm_file"
        fi
    else
        obfuscated_ll_file="${ll_file%.ll}.obfuscated.ll"
        $CLANG "$obfuscated_ll_file" "${clang_args[@]}"  -Wno-unused-command-line-argument -o "$output_file"
        if [[ -z "$DEBUG" || "$DEBUG" != "1" ]]; then
            rm "$ll_file" "$obfuscated_ll_file"
        fi
    fi

else
    echo "Not a valid .c file. Passing to clang directly."
    $CLANG "$@"
fi