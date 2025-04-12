#pragma once 

#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * @brief 线程安全队列（模板类）
 * @tparam T 队列元素类型
 */
template <typename T>
class BlockQueue
{
private:
    std::queue<T> queue_;  // 队列 
    mutable std::mutex mutex_;  // 互斥锁
    std::condition_variable cond_;  // 条件变量 用于阻塞等待

public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_.notify_one();  // 通知一个等待的线程
    }

    /**
     * @brief 尝试从队列弹出数据（非阻塞）
     * @param value 弹出值的引用
     * @return 是否成功弹出
     */
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
            return false;
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /**
     * @brief 阻塞等待并弹出数据
     * @param value 弹出值的引用
     */
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]{ return !queue_.empty(); });  // 队列空时阻塞
        value = std::move(queue_.front());
        queue_.pop();
    }

    /**
     * @brief 检查队列是否为空
     * @return 是否为空
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};