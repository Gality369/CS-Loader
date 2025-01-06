import math
from tqdm import tqdm  # 导入 tqdm

# 模数 p，即 2147483647
p = 2147483647

# 计算模 p 的所有二次剩余
def find_quadratic_residues(p):
    residues = set()
    for x in tqdm(range(1, 46340), desc="计算二次剩余"):
        residue = pow(x, 2, p)  # 计算 x^2 mod p
        residues.add(residue)
    return residues

# 找到所有非二次剩余
def find_non_quadratic_residues(p, residues):
    non_residues = []
    for i in tqdm(range(1, 46340), desc="找到非二次剩余"):
        if i not in residues:
            non_residues.append(i)
    return non_residues

# 找出由非二次剩余的平方得到的二次剩余，并且只保留平方结果唯一的非二次剩余
def find_unique_squares_of_non_residues(non_residues, p):
    square_count = {}  # 记录平方结果的出现次数
    square_map = {}    # 记录生成某个平方结果的非二次剩余

    # 使用 tqdm 显示进度条
    for nr in tqdm(non_residues, desc="处理非二次剩余"):
        square = pow(nr, 2, p)  # 计算非二次剩余的平方 mod p
        if square in square_count:
            square_count[square] += 1
        else:
            square_count[square] = 1
            square_map[square] = nr  # 记录下唯一的非二次剩余

    # 只保留那些平方结果出现次数为1的
    unique_squares = {sq for sq, count in square_count.items() if count == 1}
    
    # 对应的非二次剩余
    unique_non_residues = [square_map[sq] for sq in unique_squares]
    
    return unique_non_residues, unique_squares

# 主函数
def main():
    # 找到所有的二次剩余
    print("计算所有二次剩余...")
    residues = find_quadratic_residues(p)

    # 找到所有非二次剩余
    print("找到所有非二次剩余...")
    non_residues = find_non_quadratic_residues(p, residues)
    
    # 找出由非二次剩余平方得到的二次剩余，且平方结果唯一
    print("计算由非二次剩余的平方得到的二次剩余...")
    unique_non_residues, unique_squares = find_unique_squares_of_non_residues(non_residues, p)
    
    # 输出结果
    print(f"平方结果唯一的非二次剩余数目: {len(unique_non_residues)}")
    print(f"这些非二次剩余为: {unique_non_residues}")
    print(f"这些对应的二次剩余为: {unique_squares}")

if __name__ == "__main__":
    main()
