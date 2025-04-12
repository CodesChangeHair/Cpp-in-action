#pragma once 

#include <string>

/// 日志的输出接口(纯虚函数)，输出的设备

class LogOutput
{
public:
    /// 日志输出
    /// @param 格式化后的日志内容
    virtual void Output(
        const std::string& log
    ) = 0;

    virtual ~LogOutput() { }
};