#pragma once 

#include <string>

// LogFormat 日志格式化接口类 
// 纯虚函数作为接口

class LogFormat 
{
public:
    //////////////////////////
    /// 格式化日志转为字符串
    /// @param level 日志级别
    /// @param log 日志内容
    /// @param file 文件路径
    /// @param  line 
    // line可以不用引用优化，int的大小比指针还要小
    virtual std::string Format(
        const std::string& level,
        const std::string& log,
        const std::string& file,
        int line
    ) = 0; 

    virtual ~LogFormat() { }
};