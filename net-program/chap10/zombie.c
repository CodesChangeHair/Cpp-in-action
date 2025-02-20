#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == -1) {
        printf("fork failed\n");
        exit(1);
    } else if (pid == 0) {
        printf("Hi, I am child process");
    } else {
        printf("Child process ID: %d\n", pid);
        sleep(30);
    }

    if (pid == 0) 
        puts("End child process");
    else 
        puts("End parent process");
    
    return 0;
}