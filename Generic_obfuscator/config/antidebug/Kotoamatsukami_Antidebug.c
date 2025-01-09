#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void KObfucator_Antidebug1() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        printf("Debugger detected!\n");
        _exit(1);
    }
    printf("No debugger detected, continuing...\n");
}

void KObfucator_Antidebug2() {
    pid_t ppid = getppid();
    pid_t sid = getsid(getpid());

    if (sid != ppid) {
        printf("Debugger detected based on session and parent PID mismatch!\n");
        _exit(1);
    } else {
        printf("No debugger detected, continuing...\n");
    }
}

void KObfucator_Antidebug3() {
    signal(SIGTRAP, SIG_IGN);
    __asm__("nop\n\t"
            "int3\n\t");
    printf("No debugger detected, continuing...\n");
}

void KObfucator_Antidebug4() {
    const char *last_cmd = getenv("_");
    if (last_cmd == NULL) {
        printf("No previous command found, continuing...\n");
        return;
    }

    if (strcmp(last_cmd, "gdb") == 0 || strcmp(last_cmd, "strace") == 0 || strcmp(last_cmd, "ltrace") == 0) {
        printf("Debugger detected by environment variable!\n");
        _exit(1);
    }

    printf("No debugger detected, continuing...\n");
}
void alarm_handler(int signo) {
    printf("Debugger detected due to delayed signal handling!\n");
    _exit(1);  // 退出程序
}
void KObfucator_Antidebug5() {
    signal(SIGALRM, alarm_handler); 
    alarm(300);
}


void KObfucator_Antidebug6() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        printf("Debugger detected by ptrace!\n");
        _exit(1);
    }
    printf("No debugger detected, continuing...\n");
}

void KObfucator_Antidebug7() {
    pid_t ppid = getppid();
    char name[1024];
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", ppid);

    FILE *fp = fopen(path, "r");
    if (fp) {
        fgets(name, sizeof(name), fp);
        fclose(fp);
    }

    if (strstr(name, "gdb") || strstr(name, "strace") || strstr(name, "ltrace")) {
        printf("Debugger detected by parent process!\n");
        _exit(1);
    }

    printf("No debugger detected, continuing...\n");
}

// int main() {
//     // KObfucator_Antidebug1();  // ptrace 检测
//     // KObfucator_Antidebug2();  // getsid/getppid 检测
//     // KObfucator_Antidebug3();  // int3 检测
//     // KObfucator_Antidebug4();  // $_ 环境变量检测
//     KObfucator_Antidebug5();  // alarm 定时器检测
//     // KObfucator_Antidebug6();  // ptrace 再次检测
//     // KObfucator_Antidebug7();  // 父进程检测

//     printf("Program running normally!\n");
//     return 0;
// }
