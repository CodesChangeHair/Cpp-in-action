#pragma once 

#include "log_output.h"

#include <string>

class LogConsoleOutput
: public LogOutput
{
public:
    /// 日志输出
    /// @param 格式化后的日志内容
    void Output(
        const std::string& log
    ) override;
};