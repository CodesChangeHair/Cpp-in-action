// #include "xlog_format.h"
// #include "log_console_output.h"
// #include "log_file_output.h"
// #include "logger.h"

#include "log_factory.h"

#include <string>
#include <iostream>

using namespace std;

int main()
{
    // XConfig config;
    // config.Read();
    LogFactory::Instance().init();
    LOG_DEBUG("debug");
    LOG_INFO("info");
    LOG_ERROR("error");
    LOG_FATAL("fatal");

    // XLogFormat* log_format = new XLogFormat();
    // LogConsoleOutput* log_console_output = new LogConsoleOutput();


    // Logger logger;
    // logger.SetLogFormart(log_format);
    // logger.SetLogOutput(log_console_output);

    // logger.SetLogLevel(LogLevel::DEBUG);
    // logger.Write(LogLevel::DEBUG, "debug log info", __FILE__, __LINE__);
    // logger.Write(LogLevel::INFO, "info log info", __FILE__, __LINE__);
    // logger.Write(LogLevel::ERROR, "error log info", __FILE__, __LINE__);
    // logger.Write(LogLevel::FATAL, "fatal log info", __FILE__, __LINE__);
}