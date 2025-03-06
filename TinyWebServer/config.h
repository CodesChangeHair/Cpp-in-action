#ifndef CONFIG_H
#define CONFIG_H

#include "webserver.h"

using namespace std;

class Config 
{
public:
    Config();
    ~Config() {};

    void parse_arg(int argc, char *argv[]);

    int m_port;             // 端口号
    int m_log_write;        // 日志写入方式 同步/异步
    int m_trig_mode;        // 触发模式 ET LT
    int m_listen_trig_mode; // 监听套接字触发方式
    int m_conn_trig_mode;   // 连接套接字触发方式
    int m_opt_linger;         // 优雅关闭连接
    int m_sql_num;            // 数据库连接池数量
    int m_thread_num;         // 线程池内的线程数
    int m_close_log;          // 是否关闭日志
    int m_actor_mode;         // 并发模型选择
};

#endif