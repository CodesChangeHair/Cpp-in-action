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
    char message1[] = "Hi";
    char message2[] = "I'm another UDP host";;
    char message3[] = "Nice to meet you";

    struct sockaddr_in your_address;
    socklen_t your_address_size;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    // SOCK_DGRAM: UDP
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
        error_handling("sock() error");
    
    // 确认服务器端地址 和TCP过程一样
    memset(&your_address, 0, sizeof(your_address));
    your_address.sin_family = AF_INET;
    your_address.sin_addr.s_addr = inet_addr(argv[1]);
    your_address.sin_port = htons(atoi(argv[2]));

    sendto(client_socket, message1, sizeof(message1), 0,
           (struct sockaddr*)&your_address, sizeof(your_address));
    sendto(client_socket, message2, sizeof(message2), 0,
           (struct sockaddr*)&your_address, sizeof(your_address));
    sendto(client_socket, message3, sizeof(message3), 0,
           (struct sockaddr*)&your_address, sizeof(your_address));
    
    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}