#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

const char *P_FIFO = "p_fifo";

int main(int argc, char *argv[])
{
    int fd;
    if (argc < 2) {
        printf("please input the write data.\n");
        exit(1);
    }

    // if (access(P_FIFO, F_OK) == 0) {  // 管道文件存在
    //     execlp("rm", "-f", P_FIFO, NULL);  // 删除管道
    //     printf("access.\n");
    // }

    // // 创建命名管道
    // if (mkfifo(P_FIFO, 0666) == -1) {
    //     printf("mkfifo failed\n");
    //     exit(1);
    // }

    fd = open(P_FIFO, O_WRONLY | O_NONBLOCK);  // O_NONBLOCK: 非阻塞方式
    if (fd == -1) {
        printf("open failed!\n");
        exit(1);
    } else {
        printf("open named pipe successful");
    }

    write(fd, argv[1], 100);  // 将 argv[1] 写进 fd 中(作为文件)
    close(fd);

    return 0;
}