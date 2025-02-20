#include <unistd.h>
#include <stdio.h>

int main()
{
    alarm(1);  // 计时 1s, 1s 后内核发送一个 SIGALARM 信号终止进程(默认处理方式)
    for (int i = 0; ; ++ i)
        printf("processing...%d\n", i);
}