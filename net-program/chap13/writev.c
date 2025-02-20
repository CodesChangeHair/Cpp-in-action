#include <stdio.h>
#include <sys/uio.h>

int main(int argc, char *argv[])
{
    struct iovec vec[2];
    char buf1[] = "ABCD";
    char buf2[] = "12345";
    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = 4;
    vec[1].iov_base = buf2;
    vec[1].iov_len = 5;

    str_len = writev(1, vec, 2);
    puts("");
    printf("Write %d bytes\n", str_len);

    return 0;
}