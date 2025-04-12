#pragma once 

#include "log_format.h"

class XLogFormat:
    public LogFormat
{
public:
    //////////////////////////
    /// 格式化日志转为字符串
    /// @param level 日志级别
    /// @param log 日志内容
    /// @param file 文件路径
    /// @param  line 
    // line可以不用引用优化，int的大小比指针还要小
    /// @return 格式化后的日志
    std::string Format(
        const std::string& level,
        const std::string& log,
        const std::string& file,
        int line
    ) override;
};