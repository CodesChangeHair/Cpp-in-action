#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

/* 
 * 血缘关系之间的进程通信，本质是继承了文件描述符. 可以访问同一对文件描述符. 
 * 管道本质是一个伪文件，一个内核管冲区. 
 * 管道是内核管理的缓冲区，用来进程间的通信.
 * pipe(pipefd[2]), 返回两个文件描述符，分别用来写入和读取.
 * 在fork()之后，父进程和子进程各自有着独立的两个文件描述符, 但是各自只使用一端，关闭另一端. 
 * 因为父进程子进程资源相互独立，因此关闭一端对另一个进程没有影响. 
 * 关闭不使用的一端是为了避免资源浪费、明确管道的用途. 
 * 
 * 对于读取的一方，管道是阻塞的，需要一直等待直到管道中有数据可读. 
 */

int main()
{
    int fd[2];  // index 0: 读, 子进程; index 1: 写, 父进程
    pid_t pid;
    int ret = pipe(fd);
    if (ret == -1) {
        perror("pipe error!");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork error!");
        exit(1);
    } else if (pid == 0) {
        // 子进程 从fd[0]读取
        close(fd[1]);
        char buf[1024];
        printf("child pid = %d\n", getpid());
        ret = read(fd[0], buf, sizeof(buf));
        write(STDOUT_FILENO, buf, ret);
    } else if (pid > 0) {
        // 父进程 从fd[1]写入
        sleep(1);  // 为了展示管道的阻塞特点 (子进程会等待父进程)
        close(fd[0]);
        printf("parent pid = %d\n", getpid());
        write(fd[1], "hello pipe\n", 11);
        wait(NULL);  // 等待子进程结束 回收子进程，避免子进程成为僵尸进程
    }
    printf("complete! pid = %d\n", getpid());
    return 0;
}