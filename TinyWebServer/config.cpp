#include "config.h"

Config::Config()
{
    // 端口号，默认 9006
    m_port = 9006;

    // 日志写入方式，默认同步写入
    m_log_write = 0;

    // 触发组合模式, 默认 监听套接字为LT, 连接套接字也为LT
    m_trig_mode = 0;
    m_listen_trig_mode = 0;
    m_conn_trig_mode = 0;

    // 优雅关闭连接，默认不使用
    m_opt_linger = 0;

    // 数据库线程池内的连接数量
    m_sql_num = 8;

    // 线程池内的线程数量
    m_thread_num = 8;

    // 关闭日志，默认不关闭
    m_close_log = 0;

    // 并发模型，默认是 proactor
    m_actor_mode = 0;
}

void Config::parse_arg(int argc, char *argv[])
{
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:";
    // getopt() 用于解析命令行参数
    // 识别以 - 或 -- 开头的选项
    // optarg 是参数内部定义的全局变量，声明在 <unistd.h> 中; extern char *optarg
    // int getopt(int argc, char *const argv[], const char *optstring)
    // 命令行参数个数(main函数的argc), 命令行参数数组(main函数的argv)
    // optstring: 选项字符串, -p, -l, -s ...
    // 选项结束返回 -1
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            m_port = atoi(optarg);
            break;
        }
        case 'l':
        {
            m_log_write = atoi(optarg);
            break;
        }
        case 'm':
        {
            m_trig_mode = atoi(optarg);
            break;
        }
        case 'o':
        {
            m_opt_linger = atoi(optarg);
            break;
        }
        case 's':
        {
            m_sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            m_thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            m_close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            m_actor_mode = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}