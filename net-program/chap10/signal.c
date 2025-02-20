#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig) 
{
    puts("Time out!");
    alarm(2);
}

void keycontrol(int sig)
{
    puts("Ctrl + C pressed");
}

int main(int argc, char *argv[])
{
    int i;

    signal(SIGALRM, timeout);
    signal(SIGINT, keycontrol);

    alarm(2);

    for (i = 0; i < 3; ++ i) {
        puts("wait ...");
        sleep(1000);
    }

    return 0;
}