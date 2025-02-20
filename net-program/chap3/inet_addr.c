#include <stdio.h>
#include <arpa/inet.h>

unsigned long Inet_addr(const char* addr)
{
    unsigned long conv_addr = inet_addr(addr);
    if (conv_addr == INADDR_NONE)
        printf("Error occurred\n");
    else 
        printf("Network ordered integer address: %#lx\n", conv_addr);
    return conv_addr;
}

int main()
{
    char *addr1 = "1.2.3.4";
    char *addr2 = "1.2.3.256";  // 256 超出1个字节的最大表示范围 0 ~ 255

    // inet_addr 点分十进制 IP 地址 --> 32位整数
    unsigned long conv_addr;
    
    conv_addr = Inet_addr(addr1);
    conv_addr = Inet_addr(addr2);

    return 0;
}