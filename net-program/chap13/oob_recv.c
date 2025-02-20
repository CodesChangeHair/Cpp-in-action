#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <fcntl.h>

const int BUF_SIZE = 100;

void error_handling(const char *message);
void urg_handler(int sig);

int serv_sock, clnt_sock;

int main(int argc, char *argv[])
{
    struct sockaddr_in clnt_addr, serv_addr;
    int str_len, state;
    socklen_t clnt_addr_size;
    struct sigaction act;
    char buf[BUF_SIZE];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = urg_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    listen(serv_sock, 5);

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

    // fcntl() 用于控制文件描述符
    // 将文件描述符 clnt_sock 的所有者更改为 进程ID为 getpid() 的进程
    fcntl(clnt_sock, F_SETOWN, getpid());
    
    state = sigaction(SIGURG, &act, 0);

    while ((str_len = recv(clnt_sock, buf, BUF_SIZE - 1, 0)) != 0) {  // not EOF
        if (str_len == -1)
            continue;  // error occur
        buf[str_len] = 0;
        puts(buf);
    }

    close(serv_sock);
    close(clnt_sock);

    return 0;
}

void urg_handler(int sig)
{
    int str_len;
    char buf[BUF_SIZE];
    str_len = recv(clnt_sock, buf, BUF_SIZE - 1, MSG_OOB);
    buf[str_len] = 0;
    printf("Urgent message: %s\n", buf);
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}