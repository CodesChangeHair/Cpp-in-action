#include "xexec.h"
#include "log_factory.h"

#include <iostream>
#include <cstdio>       // popen, pclose fgets
#include <memory>
#include <stdexcept>
#include <string>

using namespace std;

/**
 * @brief 执行系统命令并返回其输出
 * @param cmd 要执行的命令（如 "ls -l" 或 "ffmpeg -version"）
 * @return std::string 命令的标准输出内容
 * @throws std::runtime_error 如果 popen 失败或命令执行出错
 */
void XExec::RunCommand(string cmd, std::function<void(const string&)> func)
{
    cmd += " 2>&1";  // 2错误输出cerr 重定向至标准输出1
    // 使用智能指针管理 FILE* 资源，确保 pclose() 会被调用
    // pipe.get() 获取原始的 FILE* 指针
    // decltype(&pclose) 自动推导 pclose 的函数指针类型
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(cmd.c_str(), "r"),  // 以只读的方式执行命令
        pclose             // 定义资源释放函数
    );

    // 检查 popen 是否成功
    if (!pipe)
    {
        LOG_ERROR("popen() failed");
        throw std::runtime_error("popen() failed");
    }

    // 存储命令输出
    string result;
    // 逐字符读取输出

    while (true)
    {
        char c = fgetc(pipe.get());
        if (c == EOF)   break;  // 读取结束

        result += c;

        // 遇到换行符输出内容
        if (c == '\n' || c == '\r')
        {
            // cout << result  << endl;
            if (!result.empty())
            {
                if (func) 
                {
                    // 调用回调函数 处理命令行输出
                    func(result);
                }
                else 
                {
                    // 将命令行放入缓冲队列
                    outputs_.push(result);
                }
                
                result.clear();
            }
        }

        // 微小延迟（避免 CPU 占用过高）
        // usleep(1000); // 1ms 延迟
    }

    // 处理最后可能剩余的内容
    if (!result.empty()) {
        if (func) {
            func(result);
        } else {
            outputs_.push(result);
        }
    }

    running_ = false;
}

string XExec::GetOutput()
{
    string line;
    outputs_.wait_and_pop(line);
    return line;
}

void XExec::Start(const std::string& cmd, std::function<void(const string&)> func)
{
    worker_thread_ = thread(&XExec::RunCommand, this, cmd, func);
    running_ = true;
}

void XExec::Stop()
{
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}