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
    int send_sock;
    struct sockaddr_in broad_addr;
    FILE *fp;
    char buf[BUF_SIZE];
    int bcast = 1;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&broad_addr, 0, sizeof(broad_addr));
    broad_addr.sin_family = AF_INET;
    broad_addr.sin_addr.s_addr = inet_addr(argv[1]); 
    broad_addr.sin_port = htons(atoi(argv[2]));  

    setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST,
                (void*)&bcast, sizeof(bcast));
    
    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");

    // feof: 检测文件指针是否到达文件末尾，如果到达，返回1，否则返回0
    while (!feof(fp)) {
        fgets(buf, BUF_SIZE, fp);
        sendto(send_sock, buf, strlen(buf), 0, 
                (struct sockaddr*)&broad_addr, sizeof(broad_addr));
        sleep(2);
    }

    close(send_sock);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}