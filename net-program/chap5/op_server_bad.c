#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int NUM_SIZE = 100;
const int BUF_SIZE = 1024;

void error_handling(const char *message);
int get_num(char **str);
void int2string(int num, char* ptr);
int calculate(int a, int b, char op);

int main(int argc, char *argv[])
{
    int server_socket, client_socket;
    char message[BUF_SIZE];
    int numbers[NUM_SIZE];
    char *ptr, op;
    int cnt, num, result, str_len;
    int i, j;

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
        
        read(client_socket, message, BUF_SIZE);
        printf("received message: %s\n", message);

        ptr = message;
        cnt = get_num(&ptr);
        printf("distance: %ld\n", message - ptr);
        for (j = 0; j < cnt; ++ j) {
            num = get_num(&ptr);
            printf("%d th number: %d\n", j + 1, num);
            numbers[j] = num;
            printf("distance: %ld\n", message - ptr);
        }

        op = *ptr;
        printf("the operator is %c\n", op);

        result = numbers[0];
        for (j = 1; j < cnt; ++ j)
            result = calculate(result, numbers[j], op);

        printf("calculate result: %d\n", result);

        int2string(result, message);
        write(client_socket, message, strlen(message));

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

int get_num(char **str) 
{
    int num = 0;
    while (**str != ' ') {
        num = num * 10 + (**str - '0');
        ++ (*str);
        printf("pointer ++!\n");
    }
    ++ (*str);
    printf("pointer ++!\n");
    return num;
}

int calculate(int a, int b, char op)
{
    switch(op) {
        case '+': a += b; break;
        case '-': a -= b; break;
        case '*': a *= b; break;
        case '/': a /= b; break;
    }
    return a;
}

// 这里默认 num > 0
void int2string(int num, char* ptr)
{
    int i, x;
    for (i = 0; ; ++ i)
    {
        if (num == 0)
            break;
        
        int x = num % 10;
        num /= 10;

        ptr[i] = (char)(x + '0');
    }
    ptr[i] = '\0';
}