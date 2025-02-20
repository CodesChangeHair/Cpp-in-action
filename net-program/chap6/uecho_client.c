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
    int client_socket;
    char message[BUF_SIZE];
    int str_len;
    socklen_t address_size;

    struct sockaddr_in server_address, from_address;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    // SOCK_DGRAM: UDP
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
        error_handling("sock() error");

    // 确认服务器端地址 和TCP过程一样
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    address_size = sizeof(from_address);
    while (1) {
        fputs("Input message (Q or q to quit): ", stdout);
        fgets(message, sizeof(message), stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        
        sendto(client_socket, message, strlen(message), 0,
               (struct sockaddr*)&server_address, sizeof(server_address));
        
        str_len = recvfrom(client_socket, message, BUF_SIZE, 0, 
                           (struct sockaddr*)&from_address, &address_size);
        
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

