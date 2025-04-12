#include "xvideo_input.h"
#include "user_input.h"
#include "xdir.h"

#include <iostream>
#include <string>

using namespace std;

void XVideoInput::RunTask(XTask& task, const XTask::Data& data)
{
    // 执行处理函数
    task.Start(data);

    while (!task.Running())
        cout << "not running!\r" << endl;
    // 显示执行函数的处理进度
    int pre = -1, cur;
    while (task.Running())
    {
        cur = task.Progress();
        if (cur != pre)
        {
            cout << "\r" << task.Progress() << "%" << flush;
            pre = cur;
        }
        
    }
    cout << "\r" << "100%" << endl;

    task.Stop();  // 等待task开启的子线程(xexec开启的)结束，否则程序Abort
}

void XVideoInput::Start(std::unique_ptr<XTask> task)
{
    UserInput input;
    XTask::Data data;  // 存储命令参数

    input
        // 注册参数处理函数
        // & lambda 函数访问其外部内容 引用方式
        // 需要确保函数调用时Data仍在存在
        .Register("-s", [&](const string& s)  // 源文件
            {
                cout << "-s: " << s << endl;
                data.src = s;
            }
        )
        .Register("-d", [&](const string& s)    // 目标文件
            {
                cout << "-d: " << s << endl;
                data.des = s;
            }
        )
        .Register("-p", [&](const string& s) // 加密密钥
            {
                if (s.empty())
                {
                    cout << "please input secret key: >> ";
                    cin >> data.secret_key;
                }
                else 
                {
                    data.secret_key = s;
                }
                data.is_encrypt = true;
            }
        )
        .Register("-dp", [&](const string& s) // 解密密钥
            {
                if (s.empty())
                {
                    cout << "please input secret key: >> ";
                    cin >> data.secret_key;
                }
                else 
                {
                    data.secret_key = s;
                }
                data.is_encrypt = false;
            }
        )
        .Register("-b", [&](const string& s)    // 截切开始时间
            {
                data.begin_seconds = s;
            }
        )
        .Register("-e", [&](const string& s)    // 截切结束时间
            {
                data.end_seconds = s;
            }
        )
        // 注册任务处理函数
        .RegisterTask("cv", [&]{ 
            cout << "cv function" << endl; 
            cout << data.src << " " << data.des << endl;
            
            data.task_type = "cv";

            if (XDir::IsDir(data.src))
            {
                auto files = XDir::GetFiles(data.src);
                for (auto& file : files)
                {
                    XTask::Data d = data;
                    d.src = file.path;

                    if (!XDir::IsDir(data.des))
                    {
                        XDir::CreateDir(data.des);
                    }

                    d.des = data.des + "/" + file.name;
                    RunTask(*task, d);
                }
            }
            else 
            {
                RunTask(*task, data);
            }
        })
        // 注册播放视频处理函数
        .RegisterTask("play", [&]{ 
            cout << "play function" << endl; 

            data.task_type = "play";
            
            // 执行处理函数
            task->Start(data);

            task->Stop();
        })
    ;

    input.Start([&]{
        cout << "init task" << endl;
        data = XTask::Data();  // 清空上一次的内容
    });
}