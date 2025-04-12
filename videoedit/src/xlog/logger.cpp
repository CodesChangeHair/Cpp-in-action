#include "logger.h"

#include <string>

using namespace std;

void Logger::Write(
        LogLevel level,
        const std::string& log,
        const std::string& file,
        int line
    )
{
    // 过滤低级别日志
    if (loglevel_ > level)
        return;
        
    string level_str = "debug";
    switch (level)
    {
    case LogLevel::INFO:
        level_str = "info";
        break;
    case LogLevel::ERROR:
        level_str = "error";
        break;
    case LogLevel::FATAL:
        level_str = "fatal";
        break;
    default:
        break;
    }
    string formatted_output = log_format_->Format(level_str, log, file, line);
    log_output_->Output(formatted_output);
}