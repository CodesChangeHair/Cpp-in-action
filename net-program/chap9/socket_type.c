#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int tcp_socket, udp_socket;
    int socket_type;
    socklen_t optlen;
    int state;

    optlen = sizeof(socket_type);
    tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);

    printf("TCP SOCKET: %d\n", SOCK_STREAM);
    printf("UDP SOCKET: %d\n", SOCK_DGRAM);

    state = getsockopt(tcp_socket, SOL_SOCKET, SO_TYPE, 
                        (void*)&socket_type, &optlen);
    if (state != 0)
        error_handling("getsockopt() error");
    printf("Socket tcp type: %d\n", socket_type);
    
    state = getsockopt(udp_socket, SOL_SOCKET, SO_TYPE, 
                        (void*)&socket_type, &optlen);
    if (state != 0)
        error_handling("getsockopt() error");
    printf("Socket udp type: %d\n", socket_type);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
