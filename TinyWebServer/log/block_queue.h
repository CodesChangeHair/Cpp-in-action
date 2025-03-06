// 循环数组实现的阻塞队列， index = (index + 1) % size
// 线程安全，对数组操作前加上互斥锁，操作完成之后解锁
// 阻塞: pop()无数据队列 or push 满队列时，阻塞

// 这里 m_front 初始化为 -1 是否会出现问题? push() --> front() 返回 m_array[-1] ?

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"
using namespace std;

template <typename T>
class BlockQueue
{
public:
    BlockQueue(int max_size = 1000)
    {
        if (max_size <= 0)
            exit(1);
        
        m_max_size = max_size;
        m_array = new T[max_size];  
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    // 析构函数只被调用一次，有必要加锁吗?(保守起见加锁当然没问题)
    ~BlockQueue()
    {
        m_mutex.lock();
        if (m_array != NULL)
            delete m_array;
        m_mutex.unlock();
    }

    void clear()
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    bool full()
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool empty()
    {
        m_mutex.lock();
        if (m_size <= 0) 
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool front(T &value)
    {
        m_mutex.lock();
        if (m_size <= 0) 
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    bool back(T &value)
    {
        m_mutex.lock();
        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size()
    {
        int tmp;

        m_mutex.lock();
        tmp = m_size;
        m_mutex.unlock();

        return tmp;
    }

    int max_size()
    {
        int tmp;

        m_mutex.lock();
        tmp = m_max_size;
        m_mutex.unlock();

        return tmp;
    }

    // 向队列中添加元素，需要将所有使用队列的线程唤醒
    // 当有元素push进队列，相当于生产者生产了一个元素
    // 若当前没有线程等待条件变量，则唤醒无意义
    bool push(const T &item)
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            // broadcast() 唤醒正在等待条件变量的线程 (调用m_cond.wait()后)
            // 可能有线程在调用pop()后阻塞(没有元素), 此时被唤醒
            // 唤醒被挂起的消费者
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }

        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        ++ m_size;

        m_cond.broadcast();
        m_mutex.unlock();

        return true;
    }

    // pop()时如果队列为空，等待条件变量
    bool pop(T &item)
    {
        m_mutex.lock();
        // 使用 while 循环判断队列是否为空，而不是和 push 一样使用 if 
        // 是为了防止虚假唤醒 spurious wakeup -- 条件变量在没有满足预期条件时仍然被唤醒
        // 此时 pop 空数据会发生错误
        // 多个消费者竞争资源时，某一时刻 A, B都能访问资源, A先访问，此时如果是 if 语句,
        // B线程会访问不存在的资源，造成错误. 此时需要 while 继续等待资源. 
        while (m_size <= 0)
        {
            // 当重新抢到互斥锁时，pthread_con_wait()返回0
            if (!m_cond.wait(m_mutex.get()))
            {
                m_mutex.unlock();
                return false;
            }
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        -- m_size;
        m_mutex.unlock();
        return true;
    }

    // 增加了超时处理
    bool pop(T &item, int ms_timeout)
    {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        
        m_mutex.lock();
        if (m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t))
            {
                m_mutex.unlock();
                return false;
            }
        }

        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        -- m_size;
        m_mutex.unlock();
        return true;
    }

private:
    Locker m_mutex;
    Cond   m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif 