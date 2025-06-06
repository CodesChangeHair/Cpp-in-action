#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int sig)
{
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);

    if (WIFEXITED(status)) {
        printf("Removed proc id: %d\n", id);
        printf("Child send: %d\n", WEXITSTATUS(status));
    }
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

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGCHLD, &act, 0);

    pid_t pid = Fork();
    if (pid == 0) {
        puts("Hi I'm child process");
        sleep(10);
        return 12;
    } else {
        printf("Child process id: %d\n", pid);
        pid = Fork();
        if (pid == 0) {
            puts("Hi I'm child process");
            sleep(10);
            return 24;
        } else {
            int i;
            printf("Child process id: %d\n", pid);
            for (i = 0; i < 5; ++ i) {
                puts("wait ...");
                sleep(5);
            }
        }
    }
}