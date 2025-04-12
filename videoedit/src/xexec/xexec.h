#pragma once 

#include <string>
#include <memory>
#include <thread>
#include <atomic>  // 多线程原子操作
#include <functional>

#include "block_queue.h"

class XExec
{
public:
    /// 执行外部进程
    /// @param cmd 命令行
    /// @return 打开命令管道是否成功
    void RunCommand(std::string cmd, std::function<void(const std::string&)> func=nullptr);

    // 开启执行命令线程
    // func 控制台输出的回调函数，设置后命令行输出不写入缓冲队列，而是交给回调函数处理
    void Start(const std::string& cmd, std::function<void(const std::string&)> func=nullptr);

    void Stop();

    /// 任务是否仍在执行
    bool Running() { return running_; }

    /// 获取输出, 存储到参数output中
    /// 成功获取返回true
    std::string GetOutput();

    ~XExec()
    {
        Stop();  // 确保线程退出
    }

private:
    std::atomic<bool> running_{ false };  // 是否允许标志

    // 缓存队列 缓存命令行输出的内容
    BlockQueue<std::string> outputs_;

    // 任务执行线程
    std::thread worker_thread_;
};