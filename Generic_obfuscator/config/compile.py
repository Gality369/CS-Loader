import json
import os
import subprocess

# 假设你的 JSON 数据文件为 "c_functions.json"
json_file = "quadratic_equations.json"

# 读取 JSON 数据
with open(json_file, 'r') as f:
    data = json.load(f)

# 创建输出目录
output_dir = "output"
os.makedirs(output_dir, exist_ok=True)

# 处理每一个 C 函数
for i, item in enumerate(data):
    c_func = item['C-Func']
    
    # 保存每个函数到单独的 C 文件
    c_file_name = os.path.join(output_dir, f"forpass_judge{i+1}.c")
    with open(c_file_name, 'w') as c_file:
        c_file.write(c_func)
    
    # 生成 LLVM IR 文件 (.ll)
    ll_file_name = os.path.join(output_dir, f"forpass_judge{i+1}.ll")
    subprocess.run(["/home/zzzccc/llvm-17/llvm-project/bin/clang", "-S", "-emit-llvm","-Wno-implicit-function-declaration",c_file_name, "-o", ll_file_name])

print(f"LLVM IR files have been generated in the '{output_dir}' directory.")
