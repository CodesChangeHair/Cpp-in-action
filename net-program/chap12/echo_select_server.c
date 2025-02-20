#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

const int BUF_SIZE = 100;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct timeval timeout;
    fd_set reads, copy_reads;
    socklen_t addr_size;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while (1) {
        copy_reads = reads;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;

        if ((fd_num = select(fd_max + 1, &copy_reads, 0, 0, &timeout)) == -1)
            error_handling("select() error");
        
        // 无事发生
        if (fd_num == 0)
            break;
        
        addr_size = sizeof(clnt_addr);
        // 监听文件描述符的 Input 事件
        // 如果是 server_socket, 表示一个客户的建立连接请求
        // 创建一个套接字建立连接
        // 如果不是 server_socket, 说明是连接套接字发送的数据
        // 如果数据长度为0, 表示EOF, 断开连接
        // 否则将接受信息返回给客户端(Echo)
        for (i = 0; i < fd_max + 1; ++ i) {
            // file descriptor 是否在 时间集合中(是否发生事件)
            if (FD_ISSET(i, &copy_reads)) {  
                if (i == serv_sock) {  // connection request to server socket
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    printf("connected client: %d\n", clnt_sock);
                } else {  // read message
                    str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0) {  // close request (EOF)
                        FD_CLR(i, &reads);
                        close(i);
                        printf("close client: %d\n", i);
                    } else {
                        write(i, buf, str_len);  // echo
                    }
                }
            }
        }
    }
    
    close(serv_sock);
    
    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}