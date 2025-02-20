#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 1024;
const int RLT_SIZE = 4;
const int OPSZ = 4;

void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int client_socket;
    char op_msg[BUF_SIZE];
    int result, opnd_cnt, i;
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
    
    fputs("Operand count: ", stdout);
    scanf("%d", opnd_cnt);
    op_msg[0] = (char)opnd_cnt;

    for (int i = 0; i < opnd_cnt; ++ i) {
        printf("Operand %d: ", i + 1);
        scanf("%d", (int*)&op_msg[i * OPSZ + 1]);
    }

    fgetc(stdin);  // 删除缓冲区中的 \n 
    fputs("Operator: ", stdout);
    scanf("%c", op_msg[opcnt * OPSZ + 1]);

    write(client_socket, op_msg, opnd_cnt * OPSZ + 2);
    
    ret = read(client_socket, &result, RLT_SIZE);
    if (ret == -1)
        error_handling("read() error");

    printf("Operation result: %s\n", result);

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