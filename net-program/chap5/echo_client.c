#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 1024;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int client_socket;
    char message[BUF_SIZE];
    int str_len, receive_len, receive_cnt;
    struct sockaddr_in server_address;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
        error_handling("sock() error");
    
    // 拨号号码 -- 服务器 ip:port
    memset(&server_address, sizeof(server_address), 0);
    server_address.sin_family = AF_INET;    // 地址族, AF_INET: IPv4
    server_address.sin_addr.s_addr = inet_addr(argv[1]);  // inet_addr 将点分十进制IP地址 --> 整数形式
     // htons(): host to network short 字节序转换Endian Conversion
     // 从主机字节序 Host Byte Order 到网络字节序 Network Byte Order 
    server_address.sin_port = htons(atoi(argv[2]));  // atoi: string --> integer, htons 字节序转换

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("connect() error");
    else 
        printf("Connected ...\n");
    
    while (1) {
        fputs("Input message (Q to quit): ", stdout);
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        
        // str_len: 传送给服务器端的字符串长度
        str_len = write(client_socket, message, strlen(message));
        
        receive_len = 0;
        while (receive_len < str_len) {
            receive_cnt = read(client_socket, message, BUF_SIZE - 1);
            if (receive_cnt == -1)
                error_handling("read() error");
            receive_len += receive_cnt;
        }
        
        message[receive_len] = 0;

        printf("Message from server: %s\n", message);
    }

    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}