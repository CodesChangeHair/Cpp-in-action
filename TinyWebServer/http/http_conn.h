#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
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
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

class HttpConn
{
public:
    // 读取文件的名称 m_real_file 的大小
    static const int FILENAME_LEN = 200;
    // 读缓冲区 m_read_buf 的大小
    static const int READ_BUFFER_SIZE = 2048;
    // 写缓冲区 m_write_buf 的大小
    static const int WRITE_BUFFER_SIZE = 1024;

    // 报文请求方法，本项目只用到了 GET 和 POST
    enum METHOD 
    {
        GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATH
    };

    // 主状态机状态
    enum CHECK_STATE 
    {
        CHECK_STATE_REQUESTLINE = 0,    // 解析请求行
        CHECK_STATE_HEADER,             // 解析请求头
        CHECK_STATE_CONTENT             // 解析消息体，仅用于解析POST请求
    };

    // 报文解析的结果
    enum HTTP_CODE 
    {
        NO_REQUEST = 0,     // 请求不完整，需要继续读取请求报文数据. 跳转到主线程(负责I/O)继续监测读事件
        GET_REQUEST,        // 获得了完整的 http 请求; 调用 do_request() 
        BAD_REQUEST,        // http 请求报文有语法错误; 跳转至 process_write() 完成报文响应
        NO_RESOURCE,
        FORBIDDEN_REQUEST,  // 请求资源禁止访问，没有读取权限; 跳转至 process_write() 完成报文响应
        FILE_REQUEST,       // 请求资源可以正常访问; 跳转至 process_write() 完成报文响应
        INTERNAL_ERROR,     // 服务器内部错误
        CLOSED_CONNECTION
    };

    // 从状态机状态
    enum LINE_STATUS 
    {
        LINE_OK = 0,    // 完整解析一行
        LINE_BAD,       // 报文语法有误
        LINE_OPEN       // 读取的行不完整
    };

public:

    HttpConn() {}
    ~HttpConn() {}

    // 初始化套接字，函数内部会调用私有函数init()
    void init(int sockfd, const sockaddr_in &addr, char *root, int trig_mode,
                    int close_log, string user, string password, string sql_name);
    // 关闭 http 连接
    void close_conn(bool real_close = true);
    void process();
    // 读取游览器发来的全部数据
    bool read_once();
    // 响应报文写入
    bool write();

    sockaddr_in *get_address()
    {
        return &m_address;
    }
    // 同步线程初始化数据库取表
    void init_mysql_result(ConnectionPool *conn_pool);

    int m_timer_flag;
    // 用于同步主线程和子线程 作为异步处理完成标志
    // improv = 1, 表示子进程事件处理完毕，主线程可以进行下一步操作
    // 主线程不阻塞等待工作线程，通过improv异步通知，提升事件循环效率
    int m_improv;  

private:
    void init();
    // 从读缓冲区 m_read_buf 读取，并处理请求报文
    HTTP_CODE process_read();
    // 向写缓冲区 m_write_buf 写入响应报文
    bool process_write(HTTP_CODE ret);
    // 主状态机 解析报文中的请求行
    HTTP_CODE parse_request_line(char *text);
    // 主状态机 解析报文中的请求头
    HTTP_CODE parse_headers(char *text);
    // 主状态机 解析报文中的请求内容
    HTTP_CODE parse_content(char *text);
    // 生成相应报文
    HTTP_CODE do_request();

    // m_start_line: 已经解析的字符的最大索引
    // get_line() 指向未处理的第一个字符
    char *get_line() { return m_read_buf + m_start_line; }
    
    // 从状态机 分析请求报文的一行
    LINE_STATUS parse_line();

    void unmap();
    
    //生成相应报文，生成对应的 8 个部分，以下函数均由 do_request() 调用
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_content_type();
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epoll_fd;
    static int m_user_count;
    MYSQL *m_mysql;
    int m_state;  // 0: 读; 1: 写

private:
    int m_sockfd;
    sockaddr_in m_address;
    
    // http 请求报文缓冲区
    char m_read_buf[READ_BUFFER_SIZE];
    // 读缓冲区 m_read_buf 中数据最后一个字节的下一个位置
    long m_read_idx;
    // 读缓冲区 m_read_buf 读取的位置
    long m_checked_idx;
    // 读缓冲区 m_read_buf 已经解析的字符数
    int m_start_line;

    // http 相应报文缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE];
    // 写缓冲区 m_write_buf 的长度
    int m_write_idx;

    // 主状态机 状态
    CHECK_STATE m_check_state;
    // 请求方法
    METHOD m_method;

    // 以下为存储解析后的请求报文的 6 个变量
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    long m_content_length;
    bool m_linger;

    char *m_file_address;       // 读取服务器上的文件地址
    struct stat m_file_stat;
    struct iovec m_iovec[2];    // io向量机制
    int m_iovec_count;
    int m_cgi;                  // 是否启用POST(?)
    char *m_string;             // 存储请求头数据
    int m_bytes_to_send;        // 剩余发送的字节
    int m_bytes_have_send;      // 已经发送的字节
    char *m_doc_root;

    map<string, string> m_users;
    int m_trig_mode;
    int m_close_log;

    char m_sql_user[100];
    char m_sql_password[100];
    char m_sql_name[100];
};

#endif