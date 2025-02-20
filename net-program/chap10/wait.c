#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t Fork() 
{
    pid_t pid = fork();
    if (pid == -1) {
        printf("fork failed\n");
        exit(1);
    } 
    return pid;
}

int main(int argc, char *argv[])
{
    int status;

    pid_t pid = Fork();

    if (pid == 0) {
        return 3;
    } else {
        printf("Child pid: %d\n", pid);

        pid = Fork();
        if (pid == 0) {
            exit(7);
        } else {
            printf("Child pid: %d\n", pid);

            wait(&status);
            if (WIFEXITED(status))
                printf("Child send one: %d\n", WEXITSTATUS(status));
            
            wait(&status);
            if (WIFEXITED(status))
                printf("Child send one: %d\n", WEXITSTATUS(status));
            
            sleep(30);
        }
    }

    return 0;
}