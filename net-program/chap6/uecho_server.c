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
    int server_socket;
    char message[BUF_SIZE];
    int str_len;
    socklen_t client_address_size;
    struct sockaddr_in server_address, client_address;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    server_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1)
        error_handling("UDP socket creation error");
    
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("bind() error");

    while (1) {
        str_len = recvfrom(server_socket, message, BUF_SIZE, 0,
                           (struct sockaddr*)&client_address, &client_address_size);
        
        sendto(server_socket, message, str_len, 0,
                (struct sockaddr*)&client_address, client_address_size);
    }

    close(server_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}