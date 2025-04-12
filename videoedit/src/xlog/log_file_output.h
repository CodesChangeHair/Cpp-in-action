#pragma once 

#include "log_output.h"

#include <fstream>


class LogFileOutput
: public LogOutput
{
public:
    /// 日志输出
    /// @param 格式化后的日志内容
    void Output(
        const std::string& log
    ) override;

    // 打开给定的文件路径 file
    bool Open(const std::string& file);
    
private:
    std::ofstream ofs_;
};