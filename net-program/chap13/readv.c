#include <stdio.h>
#include <sys/uio.h>

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    struct iovec vec[2];
    char buf1[BUF_SIZE] = {0,};
    char buf2[BUF_SIZE] = {0,};
    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = 5;
    vec[1].iov_base = buf2;
    vec[1].iov_len = BUF_SIZE;

    // 从标准输入 console 得到数据, 存储在vec指向的缓冲区中
    str_len = readv(0, vec, 2);
    printf("Read %d bytes\n", str_len);
    printf("First message: %s\n", buf1);
    printf("Second message: %s\n", buf2);

    return 0;
}