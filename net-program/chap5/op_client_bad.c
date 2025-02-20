#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 1024;

void error_handling(const char *message);
void int2string(int num, char* ptr);
void add_message(char *message, const char *add);

int main(int argc, char *argv[])
{
    int client_socket;
    char message[BUF_SIZE], str[BUF_SIZE], op[2];
    message[0] = 0; str[0] = 0;
    int i, cnt, num, ret;
    struct sockaddr_in server_address;

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
        error_handling("sock() error");
    
    // 拨号号码 -- 服务器 ip:port
    memset(&server_address, sizeof(server_address), 0);
    server_address.sin_family = AF_INET;    // 地址族, AF_INET: IPv4
    server_address.sin_addr.s_addr = inet_addr(argv[1]);  // inet_addr 将点分十进制IP地址 --> 整数形式
     // htons(): host to network short 字节序转换Endian Conversion
     // 从主机字节序 Host Byte Order 到网络字节序 Network Byte Order 
    server_address.sin_port = htons(atoi(argv[2]));  // atoi: string --> integer, htons 字节序转换

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        error_handling("connect() error");
    else 
        printf("Connected ...\n");
    
    printf("Operand count: ");
    scanf("%d", &cnt);
    puts("");

    int2string(cnt, str);
    add_message(message, str);

    for (int i = 0; i < cnt; ++ i) {
        printf("Operand: ");
        scanf("%d", &num);
        puts("");
        int2string(num, str);
        add_message(message, str);
    }

    printf("Operator: ");
    scanf("%s", op);
    add_message(message, op);
    add_message(message, "\0");

    printf("input message: %s\n", message);


    write(client_socket, message, strlen(message));
    
    ret = read(client_socket, message, BUF_SIZE - 1);
    if (ret == -1)
        error_handling("read() error");

    printf("Operation result: %s\n", message);


    close(client_socket);

    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void int2string(int num, char* ptr)
{
    int i, x;
    for (i = 0; ; ++ i)
    {
        if (num <= 0)
            break;
        
        int x = num % 10;
        num /= 10;

        ptr[i] = (char)(x + '0');
    }
    ptr[i] = ' ';
    ptr[i + 1] = 0;
}

void add_message(char *message, const char *add)
{
    strcat(message, add);
}