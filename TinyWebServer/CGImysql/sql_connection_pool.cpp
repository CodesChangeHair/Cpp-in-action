#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

ConnectionPool::ConnectionPool()
{
    m_cur_conn = 0;
    m_free_conn = 0;
}

void ConnectionPool::init(string url, string user, string password, string database_name, int port, int max_conn, int close_log)
{
    m_url = url;
    m_port = port;
    m_user = user;
    m_password = password;
    m_database_name = database_name;
    m_close_log = close_log;

    // 创建最多max_conn条数据库连接
    // 最后 m_max_conn 为成功创建的数据库连接数
    for (int i = 0; i < max_conn; ++ i)
    {
        MYSQL *conn = NULL;
        conn = mysql_init(conn);

        if (conn == NULL) 
        {
            LOG_ERROR("MYSQL Error");
            exit(1);
        }

        conn = mysql_real_connect(conn, url.c_str(), user.c_str(), password.c_str(), database_name.c_str(), port, NULL, 0);

        if (conn == NULL) 
        {
            LOG_ERROR("MYSQL Error");
            exit(1);
        }
        // 将创建的数据库连接交给链表管理 -- 连接池
        conn_list.push_back(conn);
        ++ m_free_conn;              // 更新可使用的数据库连接数
    }

    // 将信号量初始化为连接池中的连接数
    reserve = Sem(m_free_conn);
    m_max_conn = m_free_conn;
}

// 单例模式
ConnectionPool* ConnectionPool::GetInstance()
{
    static ConnectionPool conn_pool;
    return &conn_pool;
}

// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *ConnectionPool::GetConnection()
{
    MYSQL *conn = NULL;

    if (conn_list.size() == 0)
        return NULL;
    
    reserve.wait();  // 信号量 - 1, 保证了如果数据库中没有连接，会阻塞等待

    lock.lock();     // 互斥锁加锁，连接池作为多线程竞争资源，需要同步机制保护

    conn = conn_list.front();
    conn_list.pop_front();
    -- m_free_conn;
    ++ m_cur_conn;

    lock.unlock();  // 互斥锁解锁

    return conn;
}

// 释放当前使用的连接, 将该连接返回给数据库连接池
bool ConnectionPool::ReleaseConnection(MYSQL *conn)
{
    if (conn == NULL)
        return false;
    
    lock.lock();

    conn_list.push_back(conn);
    ++ m_free_conn;
    -- m_cur_conn;

    lock.unlock();

    reserve.post();  // 信号量 + 1 -- 可用连接数 + 1

    return true;
}

// 销毁数据库连接池
void ConnectionPool::DestroyPool()
{
    lock.lock();
    if (conn_list.size() > 0)
    {
        // 遍历连接池中的所有数据库连接，关闭所有数据库连接
        list<MYSQL *>::iterator it;
        for (it = conn_list.begin(); it != conn_list.end(); ++ it)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        m_cur_conn = 0;
        m_free_conn = 0;

        // 清空 list -- 连接池
        conn_list.clear();

        lock.unlock();   // 是否多余 ?
    }
    lock.unlock();
}

// 当前空闲连接数
int ConnectionPool::GetFreeConn()
{
    return this->m_free_conn;
}

// 析构函数，销毁数据库连接池中的所有连接
ConnectionPool::~ConnectionPool()
{
    DestroyPool();
}

// RAII 管理单个数据库连接，构造函数从连接池获取一个连接，析构函数释放连接返回给连接池
ConnectionRAII::ConnectionRAII(MYSQL **sql, ConnectionPool *conn_pool)
{
    // GetConnection()返回数据库连接池中的一个连接 MYSQL*
    // 因为 参数本身是指针，想要修改一个指针参数，需要双重指针
    *sql = conn_pool->GetConnection();   
    conn_RAII = *sql;                    // 将数据库连接交给一个对象管理
    pool_RAII = conn_pool;
}

ConnectionRAII::~ConnectionRAII()
{
    // 析构函数中释放数据库连接
    pool_RAII->ReleaseConnection(conn_RAII); 
}