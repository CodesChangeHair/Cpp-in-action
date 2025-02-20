#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

const int BUF_SIZE = 30;

pid_t Fork();

int main(int argc, char *argv[])
{
    int fds[2];
    char str1[] = "Hello World!";
    char str2[] = "Sorry, Who are you?";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = Fork();

    if (pid == 0) {
        write(fds[1], str1, sizeof(str1));
        // sleep(2);
        read(fds[0], buf, BUF_SIZE);
        printf("Child process, get message: %s\n", buf);
    } else {
        read(fds[0], buf, BUF_SIZE);
        printf("Parent process, get message: %s\n", buf);
        write(fds[1], str2, sizeof(str2));
        sleep(3);
    }
    
    return 0;
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