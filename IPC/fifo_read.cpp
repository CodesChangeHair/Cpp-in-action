#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h> 

const char *P_FIFO = "p_fifo";

int main(int argc, char *argv[])
{
    char cache[100];
    int fd;
    memset(cache, 0, sizeof(cache));  // 初始化缓存
    if (access(P_FIFO, F_OK) == 0) {  // 管道文件存在
        execlp("rm", "-f", P_FIFO, NULL);  // 删除管道
        printf("access.\n");
    }

    // 有名管道 fifo
    //  提供了一个路径名与管道关联
    //  以文件形式存在于文件系统中
    // mkfifo(const char *pathname, mode_t mode)
    // 参数:
    //      pathname 指定命名管道的路径名
    //      mode: 管道权限模式，与文件的权限类似
    //      实际权限会收到umask的影响，最终权限为 mode & ~umask
    // 返回值:
    //      成功: 0; 失败: -1, 并设置errno 以指示错误类型
    // 原理:
    //      mkfifo() 会在文件系统中创建一个特殊的文件(FIFO文件)
    //      该文件不存储实际数据，而是作为进程间通信的媒介.
    // 进程间通信:
    //      一个进程可以打开 FIFO文件并写入数据
    //      另一个进程打开同一个 FIFO 文件并读取数据.
    //      数据通过内核缓冲区在进程之间传递.
    if (mkfifo(P_FIFO, 0777) < 0) {
        printf("create named pipe failed\n");
        exit(1);
    }

    fd = open(P_FIFO, O_RDONLY | O_NONBLOCK);  // 非阻塞方式打开
    while (1) {
        memset(cache, 0, sizeof(cache));
        if (read(fd, cache, 100) == 0) {  // 没有读到数据
            printf("no data\n");
        } else {
            printf("data: %s\n", cache);
        }
        sleep(1);
    }
    close(fd);
    return 0;
}