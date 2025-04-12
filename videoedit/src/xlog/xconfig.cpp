#include "xconfig.h"

#include <fstream>
#include <iostream>

using namespace std;

bool XConfig::Read(const std::string file)
{
    ifstream ifs(file);
    if (!ifs.is_open())
        return false;
    
    string line;
    while (true)
    {
        // 读取每一行配置
        getline(ifs, line);
        if (!line.empty())
        {
            auto pos = line.find('=');
            if (pos > 0)
            {
                string key = line.substr(0, pos);
                string value = line.substr(pos + 1);
                config_[key] = value;
            }
        }

        // 如果出错 或 读到文件结尾
        if (!ifs.good())
            break;
    }

    config_["null"] = "";

    return true;
}

const std::string& XConfig::Get(const std::string& key)
{
    auto c = config_.find(key);
    if (c == config_.end())
        return config_["null"];
    return c->second;
}