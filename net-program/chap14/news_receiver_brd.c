#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 100;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int recv_sock;
    struct sockaddr_in recv_addr;
    int str_len;
    char buf[BUF_SIZE];

    // 这里客户端接受信息只需要指定端口，不需要指定服务端IP
    if (argc != 2) {
        printf("Usage : %s <Port>\n", argv[0]);
        exit(1);
    }

    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 接收所有接口的消息
    recv_addr.sin_port = htons(atoi(argv[1]));    // 设置接收端口
    
    if (bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        error_handling("bind() error");

    while (1) {
        str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
        if (str_len < 0)
            break;
        buf[str_len] = 0;
        fputs(buf, stdout);
    }

    close(recv_sock);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}