# CS-Avoid-killing
CS免杀,包括python版和C版本的(经测试Python打包的方式在win10上存在bug,无法运行,Win7测试无异常

V1.0: 目前测试可以过Defender/火绒的静杀+动杀,360还没测= =不想装360全家桶了,可以自行测试

下一步开发计划:

V1.5: 加入C版本CS免杀

V2.0: 开发出UI界面

V3.0: 想办法结合更多免杀技术(想去研究下进程注入/文件捆绑,不知道免杀效果怎样)

## 依赖环境

python: 

- python2
- pyinstaller
- requests

## 安装

`git clone https://github.com/Gality369/CS-Avoid-killing.git`

## 使用

1. 先通过CS生成C格式的shellcode:` Attacks -> packages -> Payload Generator `选择一个listener然后生成一个C的shellcode(其实就是二进制代码)

2. Python版:

   > 1. 将生成的shellcode填入generator.py的shellcode变量中,填入一个Key,用于后续的RC4加密
   > 2. 运行generator.py,生成的payload自动保存在payload.txt中.
   > 3. 将payload.txt上传到你的VPS中,后续生成的加载器会从VPS中获取加密shellcode并在本地解密后执行
   > 4. 将在VPS上的payload路径填入PyLoader.py的url变量中,填入你刚刚设置的Key
   > 5. 用`python pyinstaller.py -F -w pyshellcode.py`打包文件,可执行文件在pyshellcode文件夹下的dist中,双击运行可以看到主机上线

## 注意

1. pyinstaller安装细节

   > 最后一个支持python2的版本是3.6(最新版pyinstall不再支持python2)
   >
   > 网址:https://github.com/pyinstaller/pyinstaller/releases
   >
   > 需要安装pywin32:https://github.com/mhammond/pywin32

2. 目标是什么环境就在什么环境上打包,否则可能会出现无法上线的情况,不推荐使用py3对项目进行改造,ctypes对py3的支持不太好,会有些莫名其妙的bug