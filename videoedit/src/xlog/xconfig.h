#pragma once 

#include <string>
#include <map>

// 读取配置文件

class XConfig
{
public:
    bool Read(const std::string file = "log.conf");

    const std::string& Get(const std::string& key);
private:
    std::map<std::string, std::string> config_;
};