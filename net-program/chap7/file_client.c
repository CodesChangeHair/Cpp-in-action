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
    FILE *fp;

    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in server_address;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    fp = fopen("receive.dat", "wb");
    client_socket = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    while ((read_cnt = read(client_socket, buf, BUF_SIZE)) != 0) 
        fwrite((void *)buf, 1, read_cnt, fp);
    
    puts("Received file data");
    write(client_socket, "Thank you", 10);

    fclose(fp);
    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
