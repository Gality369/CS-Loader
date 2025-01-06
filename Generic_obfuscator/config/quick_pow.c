#include <stdio.h>
#include <time.h>

unsigned int quick_pow(unsigned int base, unsigned int exp, unsigned int mod)
{
    int result = 1; 
    base = base % mod;
    if (exp != 2){
        return 0;
    }
    while (exp > 0)
    {
        if (exp % 2 == 1)
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp /= 2;
    }

    return result;
}
int main()
{
    unsigned long long base = 2, exp = 10, mod = 987654321;
    int num_trials = 1000000; // 测试次
    return 0;
}
