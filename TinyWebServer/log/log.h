#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>

#include "block_queue.h"

using namespace std;

class Log
{
public:
    // 懒汉式单例模式（线程安全的局部静态变量）
    // C++ 11 后保证只在使用局部静态变量时 初始化
    // 且在初始化过程中保证其他线程无法访问该变量
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    // 调用私有方法，用于日志的异步写入
    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
    }

    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    // 接收日志级别和格式化的可变参数
    void write_log(int level, const char *format, ...);

    // 将缓冲区中的日志数据写入文件，确保日志文件的数据同步。
    void flush();

private:
    Log();
    virtual ~Log();
    void *async_write_log()
    {
        string single_log;
        // 从阻塞队列中取出一个日志 string, 写入文件
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    BlockQueue<string> *m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位
    Locker m_mutex;
    int m_close_log; //关闭日志
};

// 日志分级 Level
// Debug，调试代码时的输出，在系统实际运行时，一般不使用。
// Warn，这种警告与调试时终端的warning类似，同样是调试代码时使用。
// Info，报告系统当前的状态，当前执行的流程或接收的信息等。
// Error和Fatal，输出系统的错误信息。

#define LOG_DEBUG(format, ...)  if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...)   if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...)   if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...)  if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif 