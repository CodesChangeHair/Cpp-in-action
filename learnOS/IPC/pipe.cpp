#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>

/*
1. What is?
    管道本质上是内核中的一个缓冲区，由OS管理，用于进程间的数据传输.
    当调用 pipe(int fd[2]) 时，操作系统会在内核空间创建一个环形缓冲区.
    管道是一种半双工通信，智能单向传输数据. 
2. 为什么管道只能在亲戚进程间通信？
    匿名管道只能在 具有共享文件描述符的进程之间通信. 

    个独立创建的进程，即使都调用 pipe()，它们的 fd[0]/fd[1] 是完全不相干的内核对象，无法通信. 

3. 关闭写端或读端后，为什么不会影响另一个进程的写/读？
    因为每个进程维护自己的文件描述符表副本，关闭你自己的 fd[0] 并不会直接影响另一个进程对该 fd 的使用. 
    对应缓冲区有引用计数，当引用计数为0时，内核才会回收资源. 
*/

int main()
{
    // pipefd[0]: read, pipefd[1]: write
    // 父进程通过pipefd[1]写入
    // 子进程通过pipefd[0]读出
    int pipefd[2];  

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();  // 创建子进程

    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]);  // 关闭写端

        char buffer[128];
        int nbytes = read(pipefd[0], buffer, sizeof(buffer));
        if (nbytes > 0) {
            buffer[nbytes] = '\0';
            std::cout << "[子进程] 接收到父进程消息: " << buffer << std::endl;
        }

        close(pipefd[0]);  // 关闭读端
    } else {
        // 父进程：向子进程发送信息
        close(pipefd[0]);  // 关闭读端

        std::string msg = "你好，我是父进程！";
        write(pipefd[1], msg.c_str(), msg.size());

        close(pipefd[1]);
        wait(nullptr);  // 等待子进程结束
    }
    return 0;
}