#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

const int BUF_SIZE = 30;

int main(int argc, char *argv[])
{
    fd_set reads, temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;

    // 初始化 fs_set 变量
    FD_ZERO(&reads);
    // 将文件描述符0 (index) 对应的位置设置为1
    // 监听standard input 的输入事件
    FD_SET(0, &reads);  // 0 is standard input (console)

    // timeout.tv_sec = 5;
    // timeout.tv_usec = 0;

    while (1) {
        // select 会清除 fd_set 变量的所有位. 为保留初始值，
        // 操作temps 而不是 reads 
        temps = reads;

        // 调用select()后, timeval 结构体的数值会被替换为超时前的剩余时间
        // 因此每次调用 select() 函数前都需要设置 timeval 变量
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // 如果在 timeval 时间内 standard input 发生待读取数据时间(有新的数据产生)
        // 返回大于0的整数, 否则超时后返回0
        result = select(1, &temps, 0, 0, &timeout);

        if (result == -1) {
            puts("select() error");
            break;
        } else if (result == 0) {
            puts("Time-out (no event occur)");
        } else {
            if (FD_ISSET(0, &temps)) {
                str_len = read(0, buf, BUF_SIZE);
                buf[str_len] = 0;
                printf("Message from console: %s\n", buf);
            }
        }
    }
    return 0;
}