#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class UtilTimer;

// 客户端连接资源
struct ClientData 
{
    sockaddr_in address;
    int sockfd;
    UtilTimer *timer;
};

// 超时时间，超时处理函数，客户端连接资源，前后指针(双指针节点)
class UtilTimer
{
public:
    UtilTimer(): prev(NULL), next(NULL) { }

    time_t expire;  // 超时时间

    void (* cb_func)(ClientData *);  // handler
    ClientData *user_data;
    UtilTimer *prev;
    UtilTimer *next;
};

class SortTimeLst
{
public:
    SortTimeLst();
    ~SortTimeLst();

    void add_timer(UtilTimer *timer);
    void adjust_timer(UtilTimer *timer);
    void del_timer(UtilTimer *timer);
    void tick();

private:
    void add_timer(UtilTimer *timer, UtilTimer *lst_head);

    UtilTimer *head;
    UtilTimer *tail;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);
    
    // 对文件描述符设置非阻塞
    int set_nonblocking(int fd);

    // 在epoll内核时间表中注册 套接字 fd; 判断是否设置为 ET 模式, 是否开启 EPOLLONESHOT
    void addfd(int epoll_fd, int fd, bool one_shot, int trig_mode);

    // 信号处理函数 (静态函数，否则将传入 this 指针，导致参数不符)
    static void sig_handler(int sig);

    // 设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    // 定时处理任务，重新定时以不断触发 SIGALRM 信号
    void time_handler();

    void show_error(int conn_fd, const char *info);

public:
    static int *u_pipe_fd;
    SortTimeLst m_timer_lst;
    static int u_epoll_fd;
    int m_timeslot;
};

void cb_func(ClientData *user_data);

#endif 