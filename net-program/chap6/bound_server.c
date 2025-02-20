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
    struct sockaddr_in my_address, your_address;
    socklen_t address_size;
    int str_len, i;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    server_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1)
        error_handling("socket() error");
    
    memset(&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr*)&my_address, sizeof(my_address)) == -1)
        error_handling("bind() error");
    
    address_size = sizeof(your_address);
    for (i = 0; i < 3; ++ i) {
        sleep(5);
        str_len = recvfrom(server_socket, message, BUF_SIZE, 0,
                           (struct sockaddr*)&your_address, &address_size);
        
        printf("Message: %dth %s\n", i + 1, message);
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