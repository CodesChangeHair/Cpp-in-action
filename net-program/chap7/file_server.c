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
    int server_socket, client_socket;
    FILE *fp;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    fp = fopen("file_server.c", "rb");
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) ;
    listen(server_socket, 5);

    client_address_size = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);

    while (1) {
        read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
        if (read_cnt < BUF_SIZE) {
            write(client_socket, buf, read_cnt);
            break;
        }
        write(client_socket, buf, BUF_SIZE);
    }

    // 半关闭连接 停止写入
    // 向客户端发送 EOF, 通知客户端文件传输已经完成
    // 仍然可以接受数据
    shutdown(client_socket, SHUT_WR);

    read(client_socket, buf, BUF_SIZE);
    printf("Message from client: %s\n", buf);

    fclose(fp);
    close(server_socket);
    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}