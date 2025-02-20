#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

const int BUF_SIZE = 30;
void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int i;
    struct hostent *host;
    
    if (argc != 2) {
        printf("Usage : %s <address>\n", argv[0]);
        exit(1);
    }

    host = gethostbyname(argv[1]);

    if (!host)
        error_handling("gethostbyname() error");
    
    printf("Official name: %s\n", host->h_name);

    for (i = 0; host->h_aliases[i]; ++ i)
        printf("Aliases %d: %s\n", i + 1, host->h_aliases[i]);

    printf("Address type: %s\n", 
            (host->h_addrtype == AF_INET) ? "IPv4" : "IPv6");
    
    for (i = 0; host->h_addr_list[i]; ++ i)
        printf("IP address %d: %s\n", i + 1,
                inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
