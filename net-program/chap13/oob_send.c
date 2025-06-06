#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 100;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in recv_addr;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("sock() error");
    
    // 拨号号码 -- 服务器 ip:port
    memset(&recv_addr, sizeof(recv_addr), 0);
    recv_addr.sin_family = AF_INET;    // 地址族, AF_INET: IPv4
    recv_addr.sin_addr.s_addr = inet_addr(argv[1]);  // inet_addr 将点分十进制IP地址 --> 整数形式
     // htons(): host to network short 字节序转换Endian Conversion
     // 从主机字节序 Host Byte Order 到网络字节序 Network Byte Order 
    recv_addr.sin_port = htons(atoi(argv[2]));  // atoi: string --> integer, htons 字节序转换

    if (connect(sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        error_handling("connect() error");
    
    write(sock, "123", strlen("123"));
    send(sock, "4", strlen("4"), MSG_OOB);
    write(sock, "567", strlen("567"));
    send(sock, "890", strlen("890"), MSG_OOB);

    close(sock);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}