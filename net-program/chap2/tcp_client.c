#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in server_address;
    char message[30];
    int str_len = 0;
    int idx = 0, read_len = 0;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // PF_INET: IPv4
    // SOCK_STREAM: 面向连接的套接字 
    client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (client_socket == -1) 
        error_handling("socket() error");
    
    // 拨号号码 -- 服务器 ip:port
    memset(&server_address, sizeof(server_address), 0);
    server_address.sin_family = AF_INET;    // 地址族, AF_INET: IPv4
    server_address.sin_addr.s_addr = inet_addr(argv[1]);  // inet_addr 将点分十进制IP地址 --> 整数形式
     // htons(): host to network short 字节序转换Endian Conversion
     // 从主机字节序 Host Byte Order 到网络字节序 Network Byte Order 
    server_address.sin_port = htons(atoi(argv[2]));  // atoi: string --> integer, htons 字节序转换

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) 
        error_handling("connect() error");
    
    // 循环每次读取 1 个字节, 如果 read_len 返回0 退出
    while (read_len = read(client_socket, &message[idx ++], 1) > 0) {
        if (read_len == -1)
            error_handling("read() error");
        
        str_len += read_len;
    }

    printf("Message from server: %s\n", message);
    printf("Function read call count: %d\n", str_len);

    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}