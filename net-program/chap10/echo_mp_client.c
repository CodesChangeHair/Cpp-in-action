#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 30;

void error_handling(const char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
pid_t Fork();

int main(int argc, char *argv[])
{
    int sock;
    pid_t pid;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("sock() error");
    
    // 拨号号码 -- 服务器 ip:port
    memset(&serv_addr, sizeof(serv_addr), 0);
    serv_addr.sin_family = AF_INET;    // 地址族, AF_INET: IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  // inet_addr 将点分十进制IP地址 --> 整数形式
     // htons(): host to network short 字节序转换Endian Conversion
     // 从主机字节序 Host Byte Order 到网络字节序 Network Byte Order 
    serv_addr.sin_port = htons(atoi(argv[2]));  // atoi: string --> integer, htons 字节序转换

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    
    pid = Fork();
    if (pid == 0) 
        write_routine(sock, buf);
    else 
        read_routine(sock, buf);
    
    close(sock);

    return 0;
}

void read_routine(int sock, char *buf)
{
    while (1) {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0)
            return;
        
        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char *buf)
{
    while (1) {
        fputs("Input message (Q to quit): ", stdout);
        fgets(buf, BUF_SIZE, stdin);

        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) {
            shutdown(sock, SHUT_WR);
        }

        write(sock, buf, strlen(buf));
    }
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

pid_t Fork() 
{
    pid_t pid = fork();
    if (pid == -1) {
        printf("fork failed\n");
        exit(1);
    } 
    return pid;
}