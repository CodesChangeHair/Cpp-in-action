#include "log_file_output.h"

#include <fstream>

bool LogFileOutput::Open(const std::string& file)
{
    ofs_.open(file, std::ios::app);
    return ofs_.is_open();
}

void LogFileOutput::Output(
        const std::string& log
    ) 
{
    ofs_ << log << std::endl;
}