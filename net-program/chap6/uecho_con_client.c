#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 30;
void error_handling(const char *message);

int main(int argc, char *argv[])
{
    // connect() 连接UDP套接字，持续向同一个目标主机发送多次UDP数据报
    int client_socket;
    char message[BUF_SIZE];
    int str_len;
    socklen_t address_size;     // 多余变量 此时不需要调用sendto(), 而是 write()

    struct sockaddr_in server_address, from_address;  // 不再需要 from_address, 此时from_address = server_address

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    client_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
        error_handling("sock() error");

    // 确认服务器端地址 和TCP过程一样
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // 向UDP套接字注册目标IP和端口信息
    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    while (1) {
        fputs("Input message (Q or q to quit): ", stdout);
        fgets(message, sizeof(message), stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        /*
        sendto(client_socket, message, strlen(message), 0,
        (struct sockaddr*)&server_address, sizeof(server_address));
        */

        write(client_socket, message, strlen(message));

        /*
        str_len = recvfrom(client_socket, message, BUF_SIZE, 0, 
                    (struct sockaddr*)&from_address, &address_size);
        */

        str_len = read(client_socket, message, sizeof(message) - 1);
        message[str_len] = 0;

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
