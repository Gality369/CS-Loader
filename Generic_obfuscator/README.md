# KObfucator

KObfucator is an obfuscator based on LLVM-17, utilizing LLVM's new pass to implement plug-in features, for obfuscating multiple languages and platforms.

The name "KObfucator" is derived from my favorite anime, **Naruto**, which features the best genjutsu in my heart.

I will provide a complete set of related documentation in the future,now you can find the dynamically link files-KObfucator.so in /bin/build,and the /bin/test contains the different files obsfucated by different methods.

PS:This project is written by myself out of interest, it may not be complete, if you have any questions about this project, please feel free to contact me.

## Obfuscation

The following test file is rc4 encryption algorithm(source file in the end of this readme),the ida view of the original file is as follows:

![image-20241220160418783](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220160418783.png)

Currently open obfuscation functions include:

### Loopen

A method to obfuscate control flow of the program

The performance of using Loopen only:

![image-20241220163919762](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220163919762.png)

### branch2call

Transform the br to call some function in assembly level,curently only supported X86 and X64.

The performance of using branch2call only:

![image-20241220165753884](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220165753884.png)

![image-20241220165741070](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220165741070.png)

### ForObs

Add for loop for combating dynamic execution projects such as angr.

The performance of using ForObs only:

![image-20241220170246504](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220170246504.png)

![image-20241220170303720](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220170303720.png)

### BogusControlFlow

My bogus-control-flow is based on the version of rimao (source code: https://github.com/za233/Polaris-Obfuscator/blob/main/src/llvm/lib/Transforms/Obfuscation/BogusControlFlow2.cpp)
What i have done is to modify the judging conditions from certain to possible , but it's probability
of occurrence is so liitle that it can't happen when the actual program running .And I add a local
viriables to reinforces the illusion that it can be run.

But what I want to do is that make the false block can be real excute on a very low probability.And  if the false block have excuted ,the program will find it and re-execute the true block.But the fake variable in my false block is un-alloced so that it will corrupt.Hope I will have time to finish this in the future.

The performance of using BogusControlFlow only:

![image-20241220205919801](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220205919801.png)

### Indirect_branch

This obfuscation method can turn the branch jump to an indirect jump through the register,and each block has a unique key to increase the difficulty crackers' attacks.

The performance of using Indirect_branch only:

![image-20241220205450343](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220205450343.png)

### Indirect_call

As the indirect_branch method,this method will turn partial func call to indirect call through register with different keys.

The performance of using Indirect_call only:

![image-20241220205641931](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220205641931.png)

### SplitBasicBlock

This is an easy method to split a basic block to multiple.Its purpose is to strengthen other obfuscation algorithms.

For example , the performance of using it and the Loopen:

![image-20241220205724495](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220205724495.png)

### AddJunkCode

Add assembly level junk code , currently only support X86 and X64.
The performance of using AddJunkCode only:

![image-20241220164639032](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220164639032.png)

### Flatten

This code is adapted from the Pluto project (https://github.com/DreamSoule/ollvm17)
Thanks to the contributions of our predecessors!
The performance of using Flatten only:

![image-20241220211100256](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220211100256.png)

### Substitution

This code is adapted from the Pluto project (https://github.com/bluesadi/Pluto)
source file: https://github.com/bluesadi/Pluto/blob/kanxue/Transforms/src/Substitution.cpp
I just adapted it to the LLVM-17 and the LLVM New Pass
Thanks to the contributions of our predecessors!
The performance of using Substitution only:

![image-20241220210934072](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220210934072.png)

### GVEncrypt

The method will encrypt partial global virable or global constant with inserting the func-relative dec func in the head of the func,and guard the dec func only excute once through a global virable.May I will add the relative func in the end of the func to enhance secrecy.

The performance of using GVEncrypt only:

![image-20241220212030685](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220212030685.png)

### AntiDebug

The method will insert some anti-debugging functions into the program's constructor list to be called when the program starts running:
And I try to make the anti-debugging functions configurable,see code for details.

The performance of using AntiDebug only:

![image-20241220214513409](https://zzzcccimage1.oss-cn-beijing.aliyuncs.com/img/image-20241220214513409.png)

## How to install

You can compile LLVM-17 project in by youself in your computer,then modify the CMakeLists.txt of this project to compile it.

The following are the commands I use for your reference:

```
git clone --depth 1 -b release/17.x https://github.com/llvm/llvm-project.git
mkdir build
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" -DCMAKE_BUILD_TYPE=Release -DLLVM_INCLUDE_TESTS=OFF -DLLVM_ENABLE_RTTI=ON -DLLVM_OBFUSCATION_LINK_INTO_TOOLS=
ON -DCMAKE_INSTALL_PREFIX=./build/ ../llvm-project/llvm
ninja -j8
ninja install
```

These commands will install compiled products to <your-llvmdir>/build,then your need modify the CMakeLists.txt of this project.

```
cd KObfucator
mkdir build
cd build
cmake ..
make -j
```

 finish ~~

### How to use

You need set the configuration file in `/tmp/KObfucator/KObfucator.config`,which format is as follows.

**`0`**: All functions are turned off (everything is disabled).

**`1`**: All functions are turned on (everything is enabled).

**`2`**: Enable only the functions that are already enabled (keep the enabled functions on, others unchanged).

**`3`**: Enable all functions except those that are explicitly disabled (enable all functions that are not disabled).

Then you can use the KObfucator.so as follows:

```shell
<your-clang-17> -fpass-plugin=<your-KObfucator_so>
```

`KObfucator.config`

```json
{
    "target": "X86_64",
    "loopen": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [],
        "loopen_x_list": [
            2,
            3,
            5,
            8,
            11,
            12,
            13,
            14,
            18,
            20,
            21,
            27,
            30,
            31,
            32,
            34,
            35,
            37,
            38,
            41,
            43,
            44,
            45,
            46,
            48,
            50,
            51,
            52,
            56,
            57,
            66,
            69,
            71,
            72,
            73,
            75,
            77,
            78,
            80,
            84,
            85,
            89,
            91,
            94,
            95,
            97,
            98,
            99,
            103,
            106,
            108,
            110,
            113,
            115,
            116,
            120,
            124,
            125,
            126,
            128,
            130,
            134,
            136,
            137,
            140,
            141,
            147,
            148,
            152,
            157,
            158,
            159,
            162,
            163,
            164,
            165,
            167,
            172,
            173,
            174,
            176,
            180,
            184,
            187,
            189,
            192,
            195,
            197,
            199,
            200,
            201,
            202,
            204,
            209,
            210,
            214,
            217,
            218,
            221,
            222,
            224,
            227,
            228,
            233,
            235,
            236,
            237,
            238,
            239,
            241,
            242,
            243,
            244,
            245,
            246,
            247,
            249,
            253,
            254,
            257,
            258,
            259,
            261,
            262,
            263,
            264,
            266,
            270,
            275,
            276,
            278,
            279,
            284,
            286,
            287,
            288,
            290,
            292,
            293,
            298,
            299,
            300,
            301,
            303,
            306,
            307,
            308,
            310,
            311,
            312,
            315,
            320,
            322,
            325,
            327,
            333,
            335,
            336,
            337,
            338,
            340,
            342,
            347,
            350,
            353,
            354,
            356,
            357,
            363,
            364,
            366,
            369,
            370,
            373,
            376,
            379,
            380,
            382,
            386,
            387,
            388,
            392,
            393,
            395,
            396,
            397,
            399,
            405,
            410,
            412,
            414,
            415,
            417,
            421,
            422,
            424,
            426,
            429,
            430,
            432,
            433,
            435,
            438,
            440,
            443,
            446,
            447,
            450,
            453,
            458,
            459,
            460,
            462,
            464,
            465,
            467,
            468,
            479,
            480,
            483,
            493,
            496,
            497,
            499,
            500,
            502,
            504,
            505,
            507,
            509,
            510,
            511,
            512
        ],
        "module_name": "/home/zzzccc/cxzz/KObfucator/config/quick_pow.ll"
    },
    "ForObs": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "SplitBasicBlocks": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [],
        "split number": 3
    },
    "branch2call": {
        "model": 0,
        "enable function": [
            ""

        ],
        "disable function": [
            ""
        ],
        "split number": 3
    },
    "branch2call_32": {
        "model": 0,
        "enable function": [
            ""

        ],
        "disable function": [
            ""
        ],
        "split number": 3
    },
    "Junkcode": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "Antihook": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },    
    "Antidebug": {
        "model": 1,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },    
    "indirect_branch": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },    
    "indirect_call": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "bogus_control_flow": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "substitution": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "flatten": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    },
    "gv_encrypt": {
        "model": 0,
        "enable function": [
            ""
        ],
        "disable function": [
            ""
        ]
    }
}
```



## Test

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_KEY_LENGTH 256

void confuse_key(unsigned char *key, unsigned long key_len) {
    for (unsigned long i = 0; i < key_len; i++) {
        key[i] = key[i] ^(unsigned char)(i % 256);
    }
}

void rc4_ksa(unsigned char *s, unsigned char *key, unsigned long key_len) {
    unsigned char temp;
    unsigned long i, j = 0;
    for (i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + key[i % key_len]) % 256;
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
    }
}

void rc4_prga(unsigned char *s, unsigned char *data, unsigned long data_len) {
    unsigned char temp;
    unsigned long i = 0, j = 0, t;
    
    for (unsigned long k = 0; k < data_len; k++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        
        t = (s[i] + s[j]) % 256;
        data[k] ^= s[t];
    }
}

void rc4_encrypt_decrypt(unsigned char *data, unsigned long data_len, unsigned char *key, unsigned long key_len) {
    unsigned char s[256];
    rc4_ksa(s, key, key_len);
    rc4_prga(s, data, data_len);
}

unsigned char data[512];
int main() {
    unsigned char key[MAX_KEY_LENGTH];
    
    char input[512];
    unsigned long key_len, data_len;

    printf("请输入密钥（最多 256 个字符）：");
    fgets(input, sizeof(input), stdin);
    
    key_len = strlen(input) - 1;
    memcpy(key, input, key_len);
    confuse_key(key, key_len);
    printf("请输入需要加密的数据：");
    fgets(input, sizeof(input), stdin);
    data_len = strlen(input) - 1;
    memcpy(data, input, data_len);
    printf("\n加密前的数据：%s\n", data);
    rc4_encrypt_decrypt(data, data_len, key, key_len);
    printf("加密后的数据：");
    for (unsigned long i = 0; i < data_len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
    rc4_encrypt_decrypt(data, data_len, key, key_len);
    printf("解密后的数据：%s\n", data);

    return 0;
}

```

