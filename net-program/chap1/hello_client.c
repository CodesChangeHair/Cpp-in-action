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
    int str_len;

    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
    }

    // 安装电话
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) 
        error_handling("socket() error");

    // 确认拨打号码
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // 拨打
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("connect() error");
    
    // 接受信息
    str_len = read(client_socket, message, sizeof(message) - 1);
    if (str_len == -1)
        error_handling("read() error");

    printf("Message from server : %s\n", message);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}