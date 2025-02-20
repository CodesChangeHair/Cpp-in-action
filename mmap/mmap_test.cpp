#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

int main()
{
    // 把磁盘文件放入共享内存，这样可以使用指针访问磁盘文件
    int fd = open("test.txt", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open file error!\n");
        exit(1);
    }

    // 确保文件大小足够
    // 如果文件没有内容 即大小为0， 会导致 mmap 映射的内存区域无效
    // 返回 Bus error(core dumped)
    if (ftruncate(fd, 64) == -1) {
        printf("ftruncate error!\n");
        close(fd);
        exit(1);
    }

    // 申请共享内存映射
    void *p = mmap(NULL, 64, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap error!\n");
        exit(1);
    }

    strcpy((char *)p, "hello mmap");

    int ret = munmap(p, 64);  // 释放共享内存
    if (ret == -1) {
        perror("munmap error!\n");
        exit(1);
    }

    close(fd);
    
    return 0;
}