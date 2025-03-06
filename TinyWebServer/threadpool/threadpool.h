#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <cassert>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class ThreadPool
{
public:
    // thread_number: 线程池中线程的数量; max_requests: 请求队列中最多允许的、等待处理的请求数量
    // conn_pool: 数据库连接池指针
    ThreadPool(int actor_mode, ConnectionPool *conn_pool, int thread_number = 8, int max_request = 10000);
    ~ThreadPool();

    // 向请求队列中插入请求任务
    bool append(T *request, int state);
    bool append_p(T *request);

private:
    // 工作线程运行的函数
    static void *worker(void *arg);
    void run();     // 工作线程从请求队列中取出某个任务进行处理

private:
    int m_thread_number;    // 线程池中的线程数
    int m_max_requests;     // 请求队列中允许的最大请求数
    pthread_t *m_threads;   // 线程池数组, 大小为 m_thread_number
    std::list<T *> m_work_queue;  // 请求队列
    Locker m_queue_locker;  // 保护请求队列的互斥锁
    Sem m_queue_stat;       // 信号量 -- 任务数量，表示是否有任务需要处理
    ConnectionPool *m_conn_pool;  // 数据库连接
    int m_actor_mode;       // 模型切换
};

template <typename T>
ThreadPool<T>::ThreadPool(int actor_mode, ConnectionPool *conn_pool, int thread_number, int max_requests)
    : m_actor_mode(actor_mode), m_conn_pool(conn_pool), m_thread_number(thread_number), m_max_requests(max_requests),
      m_threads(NULL)
{
    if (thread_number < 0 || max_requests <= 0)
    {
        printf("thread number: %d, max_request: %d", thread_number, max_requests);
        throw std::exception();
    }
    
    m_threads = new pthread_t[m_thread_number];  // 线程池, 数组实现
    if (!m_threads)
    {
        printf("new pthread_t error");
        throw std::exception();
    }
    
    for (int i = 0; i < m_thread_number; ++ i)
    {
        // 创建线程出错
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            printf("pthread_create() error");
            delete[] m_threads;
            throw std::exception();
        }
        // 将线程标记为分离状态 detached state, 分离状态的线程在终止时会自动释放资源
        // 无需其他线程调用 pthread_join()来回收资源.
        // 分离状态的线程无法被 pthread_join()连接
        if (pthread_detach(m_threads[i]) != 0)
        {
            printf("pthread_detach() error");
            delete[] m_threads;
            throw std::exception();
        }
    }
}

// 析构函数，释放线程池
template <typename T>
ThreadPool<T>::~ThreadPool()
{
    delete[] m_threads;
}

template <typename T>
bool ThreadPool<T>::append(T *request, int state)
{
    m_queue_locker.lock();
    // 工作队列已满
    if (m_work_queue.size() >= m_max_requests)
    {
        m_queue_locker.unlock();
        return false;
    }

    request->m_state = state;
    m_work_queue.push_back(request);
    
    m_queue_locker.unlock();
    m_queue_stat.post();  
}

template <typename T>
bool ThreadPool<T>::append_p(T *request)
{
    m_queue_locker.lock();
    if (m_work_queue.size() >= m_max_requests)
    {
        m_queue_locker.unlock();
        return false;
    }

    m_work_queue.push_back(request);
    m_queue_locker.unlock();
    m_queue_stat.post();

    return true;
}

template <typename T>
void* ThreadPool<T>::worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
    while (true)
    {
        m_queue_stat.wait();
        m_queue_locker.lock();
        if (m_work_queue.empty())
        {
            m_queue_locker.unlock();
            continue;
        }

        T *request = m_work_queue.front();
        m_work_queue.pop_front();
        m_queue_locker.unlock();

        if (!request)
            continue;
        
        // reactor: 主线程仅注册事件，IO操作由子线程完成
        // 这里子线程需要将http报文保存至缓冲区，解析报文，将响应报文写入缓冲区
        if (m_actor_mode == 1)
        {
            if (request->m_state == 0)  // 读事件
            {
                // 循环读取客户数据，直到无数据 或 对方关闭连接(存入缓冲区中)
                if (request->read_once())
                {
                    request->m_improv = 1;
                    ConnectionRAII mysql_conn(&request->m_mysql, m_conn_pool);
                    // 处理 http 报文的解析和响应
                    // 从读缓冲区读，将响应报文写入写缓冲区
                    request->process();     
                }
                else  // 处理失败
                {
                    request->m_improv = 1;
                    request->m_timer_flag = 1;
                }
            }
            else    
            {
                if (request->write())   // 完成报文响应
                {
                    request->m_improv = 1;
                }
                else 
                {
                    request->m_improv = 1;
                    request->m_timer_flag = 1;    // 任务处理失败，将事件从时间链表中删除
                }
            }
        }
        // proactor: 主线程完成IO，子线程只需要处理业务，将响应报文写入缓冲区
        else 
        {
            ConnectionRAII mysql_conn(&request->m_mysql, m_conn_pool);
            request->process();
        }
    }
}

#endif
