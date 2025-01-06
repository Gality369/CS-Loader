#include "stdio.h"

int main(int argc, char const* argv[]) {
    __asm 
    (
        "call label\n"
        "label:\n"
        "addq $8, (%rsp)\n"  
        "ret\n"
        ".byte 0xe8\n"
        ".byte 0x90\n"
        ".byte 0x90\n"
        ".byte 0x90\n"
    );
    __asm 
    (
        "xorq %rax, %rax\n"
        "testq %rax, %rax\n"
        "jne LABLE1\n"
        "je LABLE2\n"
        "LABLE1:\n"
        ".byte 0x5e\n"
        "LABLE2:\n"
    );
    __asm 
    (
        "xorq %rax, %rax\n"
        "testq %rax, %rax\n"
        "jne LABLE3\n"
        "je LABLE4\n"
        "LABLE3:\n"
        ".byte 0x50\n"
        "LABLE4:\n"
    );
    __asm 
    (
        "xorq %rax, %rax\n"
        "testq %rax, %rax\n"
        "jne LABLE5\n"
        "je LABLE6\n"
        "LABLE5:\n"
        ".byte 0x5e\n"
        "LABLE6:\n"
    );
    __asm 
    (
        "push %rbx\n"
        "xorq %rbx, %rbx\n"
        "testq %rbx, %rbx\n"
        "jne LABLE7\n"
        "je LABLE8\n"
        "LABLE7:\n"
        ".byte 0x21\n"
        "LABLE8:\n"
        "pop %rbx\n"
    );
    
    printf("Hello world!\n");
    return 0;
}
