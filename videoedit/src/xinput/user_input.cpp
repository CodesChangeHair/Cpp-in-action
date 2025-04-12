#include "user_input.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

// static 函数，只在当前文件内(编译单元内)有效
// 将 string 按空格分割，存储在vector
static vector<string> SplitString(const string& s)
{
    istringstream is(s);
    string token;
    vector<string> vec;
    // >> 运算会自动跳过空格
    // 一个更通用的方式是使用getline(ss, token, delimiter)
    // 按照 字符(char) delimeter 分割字符串
    while (is >> token)
    {
        vec.push_back(token);
    }
    return vec;
}

void UserInput::Start(std::function<void()> init_function)
{
    while (!is_stop_)
    {
        cout << "\n>> " << flush;
        // 接收用户一行输入
        string input;
        getline(cin, input);

        // 忽略空串(用户回车)
        if (input.empty())
            continue;

        if (input == "exit")
        {
            is_stop_ = true;
            break;
        }
        
        // 1. 初始化任务 
        init_function();  // 传递而来，默认为空函数
        
        // 2. 参数分析
        // example: cv -s test.mp4 -d t.avi
        // 第一个字符(空格分隔)：元数据 命令类型
        // -s key
        // -d value

        // 先按照空格分割字符串
        auto vec = SplitString(input);
        auto& task_type = vec[0];  // 操作类型
        for (int i = 0; i < vec.size(); ++ i)
        {
            // 判断是否是关键字 目前支持: -s -d
            auto& key = vec[i];
            auto item = key_functions_.find(key);
            if (item != key_functions_.end())
            {
                if (i + 1 < vec.size()) 
                {
                    auto& value = vec[i + 1]; 
                    // 判断是否仍是关键字
                    // 例如 cv -s input.mp4 -p -d output.avi
                    // -p 后没有字符串value

                    // 若不为参数 (不在参数-回调函数 表中)
                    if (key_functions_.find(value) == key_functions_.end())
                    {
                        // 调用函数对象
                        item->second(value);
                        ++ i;
                        continue;  // 处理完毕
                    }
                }

                // 没有参数的情况
                item->second("");
            }
        }

        // 3. 执行任务
        auto task = task_functions_.find(task_type);
        // 没有该任务
        if (task == task_functions_.end())
        {
            cerr << task_type << " not supported" << endl;
        }
        else 
        {
            // 调用对应的任务处理函数
            task->second();
        }
    }   
}