#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class ConnectionPool
{
public:
    MYSQL *GetConnection();                 // 获取数据库连接
    bool ReleaseConnection(MYSQL *conn);    // 释放连接
    int GetFreeConn();                      // 获取连接
    void DestroyPool();                     // 销毁所有连接

    // 单例模式
    static ConnectionPool *GetInstance();

    void init(string url, string user, string password, string database_name, int port, int max_conn, int close_log);

private:
    ConnectionPool();
    ~ConnectionPool();

    int m_max_conn;  // 最大连接数
    int m_cur_conn;  // 当前连接数
    int m_free_conn; // 当前空闲连接数
    Locker lock;
    list <MYSQL *> conn_list;  // 连接池
    Sem reserve;

public:
    string m_url;       // 主机地址
    string m_port;      // 数据库端口号
    string m_user;      // 登录数据库用户名
    string m_password;  // 登录数据库密码
    string m_database_name;  // 数据库名
    int m_close_log;    // 日志开关
};

class ConnectionRAII
{
public:
    ConnectionRAII(MYSQL **conn, ConnectionPool *conn_pool);
    ~ConnectionRAII();

private:
    MYSQL *conn_RAII;
    ConnectionPool *pool_RAII;
};

#endif 