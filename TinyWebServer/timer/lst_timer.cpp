#include "lst_timer.h"
#include "../http/http_conn.h"

SortTimeLst::SortTimeLst() : head(NULL), tail(NULL)
{
}

SortTimeLst::~SortTimeLst()
{
    UtilTimer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void SortTimeLst::add_timer(UtilTimer *timer)
{
    if (!timer)
        return;
    
    if (!head)
    {
        head = tail = timer;
        return;
    }

    if (timer->expire < head->expire)
    {
        head->prev = timer;
        timer->next = head;
        head = timer;
    }
    else 
        add_timer(timer, head);
}

void SortTimeLst::adjust_timer(UtilTimer *timer)
{
    if (!timer)
        return;
    
    if (timer == head && timer == tail)
        return;
    
    UtilTimer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire))
        return;
    
    if (timer == head)
    {
        head = head->next;
        tmp->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head); 
    }
    else 
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, head);
    }
}

void SortTimeLst::del_timer(UtilTimer *timer)
{
    if (!timer)
        return;
    
    if (timer == head || head == tail)
    {
        if (head == tail)
        {
            delete timer;
            head = NULL;
            tail = NULL;
        }
        else if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
        }
        else if (timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
        }
    }
    else 
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }
}

void SortTimeLst::tick()
{
    if (!head)
        return;
    
    time_t cur = time(NULL);  // 当前时间
    UtilTimer *tmp = head;
    while (tmp)
    {
        // 没有事件超时了 (超时事件在之前的while循环中被删除了)
        if (tmp->expire > cur)
            break;
        
        head = tmp->next;
        tmp->cb_func(tmp->user_data);  // 调用超时处理函数，释放资源，断开连接

        if (head) 
            head->prev = NULL;
        
        delete tmp;

        tmp = head;
    }
}

// 这里只考虑 timer 插入头节点之后的情况
void SortTimeLst::add_timer(UtilTimer *timer, UtilTimer *lst_head)
{
    UtilTimer *prev = lst_head;
    UtilTimer *tmp  = prev->next;

    while (tmp)
    {
        if (timer->expire < tmp->expire)
        {
            prev->next = timer;
            timer->prev = prev;
            tmp->prev = timer;
            timer->next = timer;
            break;
        }
        else 
        {
            prev = tmp;
            tmp = tmp->next;
        }
    }

    // while 循环中没有符合 timer 的位置，timer 作为 tail 插入
    if (!tmp)
    {
        tail->next = timer;
        timer->prev = timer;
        timer->next = NULL;
        tail = timer;
    }
}

void Utils::init(int timeslot)
{
    m_timeslot = timeslot;
}

// 对文件描述符设置非阻塞
int Utils::set_nonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 向 epoll 内核事件表中注册 fd 读事件，选择开启 ET, ONESHOT; 
// 并将文件描述符设置为非阻塞模式
void Utils::addfd(int epoll_fd, int fd, bool one_shot, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    if (trig_mode == 1)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

// 信号处理函数: 将信号通过管道发送给主进程
// 缩短异步执行时间，减少信号被屏蔽的时间
void Utils::sig_handler(int sig)
{
    // 为保证函数的可重入新，保留原来的 errno
    // 可重入性: 中断后再次进入该函数，环境变量与之前相同
    int save_errno = errno;
    int msg = sig;

    // 将信号从管道写端写入，传输字符类型
    send(u_pipe_fd[1], (char *)&msg, 1, 0);

    // reload 
    errno = save_errno;
}

// 设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    
    // 将所有信号加入到信号集中 (?)
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

// 定时调用计时器链表监测超时事件
// 重新设定定时信号，周期性触发事件
void Utils::time_handler()
{
    m_timer_lst.tick();
    alarm(m_timeslot);
}

void Utils::show_error(int conn_fd, const char *info)
{
    send(conn_fd, info, strlen(info), 0);
    close(conn_fd);
}

// 静态变量类外初始化
int *Utils::u_pipe_fd = 0;
int Utils::u_epoll_fd = 0;

class Utils;
void cb_func(ClientData *user_data)
{
    epoll_ctl(Utils::u_epoll_fd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    -- HttpConn::m_user_count;
}