#include "log_factory.h"
#include "log_console_output.h"
#include "log_file_output.h"
#include "xlog_format.h"
#include "xconfig.h"

#define LOGTYPE "console"
#define LOGFILE "log.txt"
#define LOGLEVEL "debug"

using namespace std;

void LogFactory::init(
        const string &config_file
    )
{
    // logger_.SetLogFormat(new XLogFormat());
    logger_.SetLogFormat(make_unique<XLogFormat>());

    string log_type = LOGTYPE;
    string log_file = LOGFILE;
    string log_level = LOGLEVEL;

    XConfig config;
    bool result = config.Read(config_file);
    if (result)
    {
        string val = config.Get("log_type");
        if (val != "")
            log_type = val;
        
        val = config.Get("log_file");
        if (val != "")
            log_file = val;

        val = config.Get("log_level");
        if (val != "")
            log_level = val;
    }

    if (log_type == "console") 
    {
        auto output = make_unique<LogConsoleOutput>();
        logger_.SetLogOutput(move(output));
    }
    else 
    {
        auto output = make_unique<LogFileOutput>();
        if (!output->Open(log_file))
            return; // 应该要抛出异常
        logger_.SetLogOutput(move(output));
    }

    LogLevel level;
    if (log_level == "debug")
    {
        level = LogLevel::DEBUG;
    }
    else if (log_level == "info")
    {
        level = LogLevel::DEBUG;
    }
    else if (log_level == "error")
    {
        level = LogLevel::ERROR;
    }
    else if (log_level == "fatal")
    {
        level = LogLevel::FATAL;
    }

    logger_.SetLogLevel(level);
}