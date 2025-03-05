# Generic_obfuscator

Generic_obfuscator is an obfuscator based on LLVM-17, utilizing LLVM's new pass to implement plug-in features, for obfuscating multiple languages and platforms.

I will provide a complete set of related documentation in the future,now you can find the dynamically link files-Generic_obfuscator.so in /bin/build.

PS:This project is written by myself out of interest, it may not be complete, if you have any questions about this project, please feel free to contact me.

**Now u can taste it through my ugly site http://39.102.210.108:8080/**

The obfuscation algorithm details in docs

## How to install

You can compile LLVM-17 project in by youself in your computer,then modify the CMakeLists.txt of this project to compile it.

The following are the commands I use for your reference:

```
git clone --depth 1 -b release/17.x https://github.com/llvm/llvm-project.git
mkdir build
cmake -G Ninja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" -DCMAKE_BUILD_TYPE=Release -DLLVM_INCLUDE_TESTS=OFF -DLLVM_ENABLE_RTTI=ON -DCMAKE_INSTALL_PREFIX=./build/ ../llvm-project/llvm
ninja -j8
ninja install
```

These commands will install compiled products to <your-llvmdir>/build,then your need modify the CMakeLists.txt of this project.

```
cd Generic_obfuscator
mkdir build
cd build
cmake ..
make -j
```

 finish ~~

## How to use

Now you can use this obfucator easily,you just need install the clang-17 first,then modify the `Generic_obfuscator_so`  in the `compiler/clang_wrapper.sh`,then  you can use it in `compiler`  directory, and i will supply compiled so in the `/bin`.

(PS:If you don't use it in `compiler`  directory,make sure to copy `/compiler/Generic_obfuscator.config`  and `/compiler/branch2call_process.py`to your `work directory`)

The compile options to use as shown below():

```sh
./clang_wrapper.sh flatten branch2call …… <input_file> -o <output_file>
```

- <input_file>: Path to the source code file you want to obfuscate (e.g., my_program.c).
- -o <output_file>: Path to the output executable file (e.g., my_program).
- {obfuscation_options}: This is a space-separated list of obfuscation passes you wish to apply. Here are the available options (matching the internal pass names in the provided code snippet):
  - **split-basic-block**: Splits basic blocks within the code.
  - **anti-debug**: Inserts anti-debugging techniques.
  - **gv-encrypt**: Encrypts global variables.
  - **bogus-control-flow**: Inserts bogus control flow to confuse analysis.
  - **add-junk-code**: Adds junk code to increase code size and complexity.
  - **loopen**: Applies loop-based obfuscation.
  - **for-obs**: Applies for loop based obfuscation
  - **branch2call-32**: Converts branches to calls (32-bit version).
  - **branch2call**: Converts branches to calls.
  - **indirect-call**: Inserts indirect function calls.
  - **indirect-branch**: Inserts indirect branches.
  - **flatten**: Flattens the control flow of the program.
  - **substitution**: Replaces instructions with equivalent sequences.

**Example:**

To apply global variable encryption and bogus control flow to a file named rc4.c, and generate an executable named rc4, you would use:

```sh
./clang_wrapper.sh gv-encrypt bogus-control-flow ./tests/rc4.c -o ./tests/rc4
```

To apply global variable encryption only:

```sh
./clang_wrapper.sh  gv-encrypt  ./tests/rc4.c -o ./tests/rc4
```

### Details

You can set the configuration file in `/tmp/Generic_obfuscator/Generic_obfuscator.config`,which format is as follows.

**`0`**: All functions are turned off (everything is disabled).

**`1`**: All functions are turned on (everything is enabled).

**`2`**: Enable only the functions that are already enabled (keep the enabled functions on, others unchanged).

**`3`**: Enable all functions except those that are explicitly disabled (enable all functions that are not disabled).

You can find the example of Generic_obfuscator.config in https://github.com/zzzcccyyyggg/Generic_obfuscator/blob/llvm-17-plugins/compiler/Generic_obfuscator.config

Then you can use the Generic_obfuscator.so as follows:

```shell
<your-clang-17> -fpass-plugin=<your-Generic_obfuscator_so>
```