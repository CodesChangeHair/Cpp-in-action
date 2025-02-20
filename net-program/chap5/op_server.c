#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 1024;
const int OPSZ = 4;

void error_handling(const char *message);
int calculate(int opnum, int opnds[], char operator);

int main(int argc, char *argv[])
{
    int server_socket, client_socket;
    char op_info[BUF_SIZE];
    int result, opnd_cnt, i;
    int receive_cnt, receive_len;

    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        error_handling("socket() error");
    
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("bind() error");
    
    if (listen(server_socket, 5) == -1)
        error_handling("listen() error");
    
    client_address_size = sizeof(client_address);

    for (i = 0; ; ++i) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
        if (client_socket == -1) 
            error_handling("accept() error");
        else 
            printf("Connected client %d\n", i + 1);
        
        opnd_cnt = 0;
        read(client_socket, &opnd_cnt, 1);

        receive_len = 0;
        while (receive_len < (opnd_cnt * OPSZ + 1)) {
            receive_cnt = read(client_socket, &op_info[recv_len], BUF_SIZE - 1);
            receive_len += receive_cnt;
        }

        result = calculate(opnd_cnt, (int*)op_info, op_info[receive_len - 1]);

        write(client_socket, (char*)result, sizeof(result));

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int calculate(int opnum, int opnds[], char op)
{
    int result = opnds[0], i;
    switch(op) {
        case '+': 
            for (i = 1; i < opnum; ++ i)
                result += opnds[i];
                break;
        case '-':
            for (i = 1; i < opnum; ++ i)
                result -= opnds[i];
            break;
        case '*': 
            for (i = 1; i < opnum; ++ i)
                result *= opnds[i];
            break;
        case '/':
            for (i = 1; i < opnum; ++ i)
                result /= opnds[i];
            break;
    }
    return result;
}