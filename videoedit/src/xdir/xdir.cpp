#include "xdir.h"

#include <filesystem>

using namespace std;

bool XDir::IsDir(const std::string& path)
{
    return filesystem::is_directory(path);
}

bool XDir::CreateDir(const std::string& path)
{
    return filesystem::create_directory(path);
}

std::vector<XFile> XDir::GetFiles(const std::string& path)
{
    vector<XFile> files;
    // 迭代目录
    auto itr = filesystem::recursive_directory_iterator(path);
    for (auto& p : itr)
    {
        // 目录 跳过
        if (p.is_directory())
            continue; 

        // 非普通文件 跳过
        if (!p.is_regular_file())
            continue;
        
        files.push_back(
            {
                p.path().filename().string(),
                p.path().string(),
                p.path().extension().string()
            }
        );
    }
    return files;
}