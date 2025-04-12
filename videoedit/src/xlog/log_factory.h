#pragma once 

#include <string>

#include "logger.h"

// logger 工厂类
// 维护 logger 的声明周期; logger builder

// 单例模式
class LogFactory
{
public:
    static LogFactory& Instance()
    {
        static LogFactory factory;
        return factory;
    }

    // 读取配置文件，根据配置文件创建日志
    void init(
        const std::string &config_file = "log.conf"
    );

    void Write(
        LogLevel level,
        const std::string& log,
        const std::string& file,
        int line
    )
    {
        logger_.Write(level, log, file, line);
    }



private:
    LogFactory() { }

    Logger logger_;
};

#define XLOGINIT() LogFactory::Instance().init()

#define LOG_DEBUG(s) LogFactory::Instance().Write(LogLevel::DEBUG, s, __FILE__, __LINE__)
#define LOG_INFO(s) LogFactory::Instance().Write(LogLevel::INFO,   s, __FILE__, __LINE__)
#define LOG_ERROR(s) LogFactory::Instance().Write(LogLevel::ERROR, s, __FILE__, __LINE__)
#define LOG_FATAL(s) LogFactory::Instance().Write(LogLevel::FATAL, s, __FILE__, __LINE__)