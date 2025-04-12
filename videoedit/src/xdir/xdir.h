#pragma once 

// #include "xlog.h"

#include <string>
#include <vector>

/// 目录处理，最少需要C++ 17

struct XFile
{
    std::string name;
    std::string path;
    std::string ext;  // 文件后缀名
};

class XDir
{
public:
    /// 获取 path 目录下的所有文件
    static std::vector<XFile> GetFiles(const std::string& path);

    static bool IsDir(const std::string& path);

    static bool CreateDir(const std::string& path);
};