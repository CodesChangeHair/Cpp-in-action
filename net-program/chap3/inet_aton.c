#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

void error_handling(const char *message);

int main()
{
    char *addr = "127.232.124.79";
    struct sockaddr_in addr_inet;

    // 将点分十进制 IP 地址转换为 32 位整数形式， 并保存至 sockaddr_in 结构体中
    if (!inet_aton(addr, &addr_inet.sin_addr))
        error_handling("inet_aton() error");
    else 
        printf("Network ordered integer addr: %#x\n",
                addr_inet.sin_addr.s_addr);
    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}