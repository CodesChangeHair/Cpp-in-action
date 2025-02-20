#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int server_socket;
    int client_socket;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_size;

    char message[] = "Hello World!";

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    // socket() 函数创建套接字 -- 安装电话
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        error_handling("socket() error");
    
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;        
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);  // ip
    server_address.sin_port = htons(atoi(argv[1]));  // port

    // 分配IP:port -- 给电话分配电话号码
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("bind() error");
    
    // 将套接字转为可接收连接状态
    if (listen(server_socket, 5) == -1) 
        error_handling("listen() error");
    
    client_address_size = sizeof(client_address);
    // 处理连接请求. 阻塞等待连接到来
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
    if (client_socket == -1)
        error_handling("accept() error");
    
    // 类似对文件的写入操作
    write(client_socket, message, sizeof(message));

    close(client_socket);
    close(server_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}