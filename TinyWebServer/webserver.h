#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./http/http_conn.h"

const int MAX_FD = 65536;            // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000;  // 最大事件数
const int TIMESLOT = 5;              // 最小超时单位

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(int port, string user, string password, string database_name,
              int log_write, int opt_linger, int trig_mode, int sql_num,
              int thread_num, int close_log, int actor_mode);
    
    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void event_listen();
    void event_loop();
    void timer(int conn_fd, struct sockaddr_in client_address);
    void adjust_timer(UtilTimer *timer);
    void del_timer(UtilTimer *timer, int sockfd);
    bool deal_client_data();
    bool deal_with_signal(bool &timeout, bool &stop_server);
    void deal_with_read(int sockfd);
    void deal_with_write(int sockfd);

public:
    // 基础信息
    int m_port;         // 服务器监听端口
    char *m_root;       // 服务器资源根目录路径
    int m_log_write;    // 日志写入方式(同步 / 异步)
    int m_close_log;    // 是否关闭日志
    int m_actor_mode;   // 并发模型(Reactor / Proactor)

    int m_pipe_fd[2];   // 用于信号通信的管道(0: 读, 1: 写)
    int m_epoll_fd;     // epoll 实例的文件描述符
    HttpConn *users;    // Http 连接对象数组，每个客户端一个，客户端socket文件描述符作为索引

    // 数据库相关
    ConnectionPool *m_conn_pool;    // 数据库连接池指针
    string m_user;      // 登录数据库用户名
    string m_password;
    string m_database_name;
    int m_sql_num;          // 数据库连接池初始连接数

    // 线程池相关
    ThreadPool<HttpConn> *m_pool;   // 线程池指针
    int m_thread_num;               // 线程池连接初始化数量

    // epoll_event 相关
    epoll_event events[MAX_EVENT_NUMBER];   // epoll 事件数组

    int m_listen_fd;                        // 监听socket的连接描述符
    int m_opt_linger;                       // 是否启用SO_LINGER(优雅关闭)
    int m_trig_mode;                        // 触发模式
    int m_listen_trig_mode;                 // 监听socket的触发模式
    int m_conn_trig_mode;                   // 连接 socket 的触发模式

    // 定时器相关
    ClientData *client_data;            // 客户端连接资源数组, 与users一一对应
    Utils utils;                        // 工具类(定时器，信号管理)
};

#endif 