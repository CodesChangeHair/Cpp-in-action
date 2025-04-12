#pragma once

#include <map>
#include <string>
#include <functional>

class UserInput
{
public:
    // 启动控制台任务，获取用户输入
    // 传入初始化函数，默认值为空函数(lambda表达式创建)
    void Start(std::function<void()> init_function = []{});

    void Stop() { is_stop_ = true; }

    // 注册参数对应的回调函数
    UserInput& Register(std::string key, 
                std::function<void (const std::string&)> func
    )
    {
        key_functions_[key] = func;
        return *this;
    }

    // 注册执行任务的函数
    UserInput& RegisterTask(std::string key, 
                std::function<void ()> func
    )
    {
        task_functions_[key] = func;
        return *this;
    }
private:
    bool is_stop_{ false };

    // 关键字映射函数表 不同的关键字对应不同的回调函数
    // string: function 
    std::map<std::string,   
             std::function<void (const std::string&)>> key_functions_;
             
    // 任务执行函数 多个
    std::map<std::string,   
             std::function<void ()>> task_functions_;
};