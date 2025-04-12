#pragma once 

#include <string>
#include <memory>

#include "log_format.h"
#include "log_output.h"

// 聚合类 委托 LogFormat 和 LogOutput
// 以实现日志的写入

// 枚举级别 支持运算
// enum class 而不是 enum
// 有命名空间限制
enum class LogLevel
{
    DEBUG,
    INFO,
    ERROR,
    FATAL
};

class Logger
{
public:
    //////////////////////////
    /// 格式化日志转为字符串
    /// @param level 日志级别
    /// @param log 日志内容
    /// @param file 文件路径
    /// @param line 
    // line可以不用引用优化，int的大小比指针还要小
    /// @return 格式化后的日志
    void Write(
        LogLevel level,
        const std::string& log,
        const std::string& file,
        int line
    );

    void SetLogOutput(std::unique_ptr<LogOutput> log_output)
    {
        log_output_ = move(log_output);
    }

    void SetLogFormat(std::unique_ptr<LogFormat> log_format)
    {
        log_format_ = move(log_format);
    }

    void SetLogLevel(LogLevel level)
    {
        loglevel_ = level;
    }

    ~Logger()
    {
        // 智能指针自动销毁 (类似RAII，用对象管理堆的生命周期)
        // delete log_output_; log_output_ = nullptr;
        // delete log_format_; log_format_ = nullptr;
    }

private:
    // 委托 调用接口 委托接口实现日志的格式化 以及 输出
    // LogFormat* log_format_{ nullptr };
    // LogOutput* log_output_{ nullptr }; 
    std::unique_ptr<LogFormat> log_format_;
    std::unique_ptr<LogOutput> log_output_;

    // 最低日志级别
    LogLevel loglevel_{ LogLevel::DEBUG };
};