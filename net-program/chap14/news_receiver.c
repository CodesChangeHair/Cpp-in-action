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
    int str_len;
    char buf[BUF_SIZE];
    struct sockaddr_in recv_addr;
    struct ip_mreq join_addr;

    if (argc != 3) {
        printf("Usage : %s <GroupIP> <Port>\n", argv[0]);
        exit(1);
    }

    // 创建一个UDP套接字，用于接收多播数据
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);

    // 初始化接收地址结构体
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 接收所有接口的消息
    recv_addr.sin_port = htons(atoi(argv[2]));    // 设置接收端口

    // 将套接字与指定的地址和端口绑定
    if (bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        error_handling("bind() error");

    // 设置多播组信息，加入指定的多播组
    join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]);  // 多播组的IP地址
    join_addr.imr_interface.s_addr = htonl(INADDR_ANY);   // 使用本地接口接收数据

    // 通过setsockopt将接收端加入到指定的多播组
    // IP_ADD_MEMBERSHIP：加入多播组
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                (void*)&join_addr, sizeof(join_addr));

    // 循环接收多播数据
    while (1) {
        // 接收数据，返回接收到的字节数
        str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
        if (str_len < 0)  // 如果接收错误，跳出循环
            break;
        buf[str_len] = 0;  // 在接收到的数据后加上字符串结束符
        printf("Received message: %s\n", buf);  // 打印接收到的消息
    }

    // 关闭套接字
    close(recv_sock);

    return 0;
}

// 错误处理函数，打印错误信息并退出
void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
