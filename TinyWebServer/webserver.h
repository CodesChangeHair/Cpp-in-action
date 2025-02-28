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

#include "/threadpool/threadpool.h"
#include "./http/http_conn.h"

const int MAX_FD = 65536;            // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000;  // 最大事件数
const int TIMESLOT = 5;              // 最小超时单位

class WebServer 
{
public:
    WebServer();
    ~WebServer();

    void init(int port, string user, string password, string databaseName,
              int log_writer, int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);
    
    void thread_pool();
    void sql_pool();
    void log_writer();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(util_timer *timer);
    void adjust_timer(util_timer *timer, int sockfd);
    bool deal_client_data();
    bool deal_with_signal(bool& timerout, bool& stop_server);
    void deal_with_thread(int sockfd);
    void deal_with_write(int sockfd);

public:
    // 基础信息
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipefd[2];
    int m_epollfd;
    http_conn *user;

    // 数据库相关
    connection_pool *m_coonPool;
    string m_user;  
    string m_password;
    string m_database_name;
    int m_sql_num;

    // 线程池相关
    thread_pool<http_conn> *m_pool;
    int m_thread_num;

    // epoll_event 相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    // 定时器相关
    client_data *user_timer;
    Utils utils;
};

#endif
