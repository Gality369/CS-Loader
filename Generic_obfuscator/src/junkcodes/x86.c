#include "stdio.h"

int main(int argc, char const* argv[]) {
    __asm 
    (
    "call label\n"
    "label:\n"
    "addl $8, (%esp)\n"
    "ret\n"
    ".byte 0xe8\n"
    ".byte 0x90\n"
    ".byte 0x90\n"
    ".byte 0x90\n"
    );
   __asm 
    (
        "xorl %eax, %eax\n"
        "testl %eax, %eax\n"
        "jne LABLE1\n"
        "je LABLE2\n"
        "LABLE1:\n"
        ".byte 0x5e\n"
        "LABLE2:\n"
    );
    __asm 
    (
        "xorl %eax, %eax\n"
        "testl %eax, %eax\n"
        "jne LABLE3\n"
        "je LABLE4\n"
        "LABLE3:\n"
        ".byte 0x50\n"
        "LABLE4:\n"
    );
    __asm 
    (
        "xorl %eax, %eax\n"
        "testl %eax, %eax\n"
        "jne LABLE5\n"
        "je LABLE6\n"
        "LABLE5:\n"
        ".byte 0x5e\n"
        "LABLE6:\n"
    );
    __asm 
    (
        "push %ebx\n"
        "xorl %ebx, %ebx\n"
        "testl %ebx, %ebx\n"
        "jne LABLE7\n"
        "je LABLE8\n"
        "LABLE7:\n"
        ".byte 0x21\n"
        "LABLE8:\n"
        "pop %ebx\n"
    );
    printf("Hello world!\n");
    return 0;
}
