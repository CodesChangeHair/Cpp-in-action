#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int TTL = 64;  // Time To Live
const int BUF_SIZE = 100;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int send_sock;
    struct sockaddr_in mul_addr;
    int time_live = TTL;
    FILE *fp;
    char buf[BUF_SIZE];

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&mul_addr, 0, sizeof(mul_addr));
    mul_addr.sin_family = AF_INET;
    // 这一行设置了多播地址（argv[1]），而不是普通的单播地址。
    // 多播地址通常位于 224.0.0.0 到 239.255.255.255 之间。
    // 通过发送到这样的地址，数据包可以被多个接收者（加入该多播组的主机）接收。
    mul_addr.sin_addr.s_addr = inet_addr(argv[1]);  // Multicast IP
    mul_addr.sin_port = htons(atoi(argv[2]));  // Multicast Port

    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL,
                (void*)&time_live, sizeof(time_live));
    
    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");
    
    while(!feof(fp)) {  // Broadcasting
        fgets(buf, BUF_SIZE, fp);
        sendto(send_sock, buf, strlen(buf), 0,
                (struct sockaddr*)&mul_addr, sizeof(mul_addr));
        sleep(2);
    }

    fclose(fp);
    close(send_sock);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}