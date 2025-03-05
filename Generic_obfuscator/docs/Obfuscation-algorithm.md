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

~~The method will insert some anti-debugging functions into the program's constructor list to be called when the program starts running:
And I try to make the anti-debugging functions configurable,see code for details~~.

Now the method wiil randomly insert some anti-debug functions into the each functions of the program to guard the program in runtime. 