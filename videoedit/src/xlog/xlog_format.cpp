#include "xlog_format.h"
#include <sstream>
#include <iomanip>      // 对输入输出流的格式化控制 std::put_time
#include <chrono>       // C++ 11引入的时间库，提供了高精度记时功能
#include <ctime>        // 提供了C语言中的时间相关功能 time_t, std::localtime
#include <cstdlib>      // 包含了与环境和程序控制相关的功能, setenv tzset 用于设置和使时区环境变量生效

using namespace std;

std::string GetFormattedTime(const std::string& format, const std::string& timezone) 
{
    // 设置时区
    setenv("TZ", timezone.c_str(), 1);
    tzset();

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 格式化时间
    std::stringstream ss;
    std::tm* tm_ptr = std::localtime(&now_c);
    ss << std::put_time(tm_ptr, format.c_str());
    return ss.str();
}

std::string GetNow()
{
    return GetFormattedTime("%Y-%m-%d %H:%M:%S", "Asia/Shanghai");
}

std::string XLogFormat::Format(
    const std::string& level,
    const std::string& log,
    const std::string& file,
    int line
) 
{
    stringstream ss;
    ss << GetNow() << " " 
       << level << " " 
       << log << " " 
       << file << ": " 
       << line;
    return ss.str();
}