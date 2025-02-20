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
        sleep(24);
        return 24;
    } else {
        while (waitpid(-1, &status, WNOHANG) == 0) {
            sleep(3);
            puts("sleep 3 sec.");
        }

        if (WIFEXITED(status))
            printf("Child send one: %d\n", WEXITSTATUS(status));
    }

    return 0;
}