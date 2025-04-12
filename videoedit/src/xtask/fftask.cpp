#include "fftask.h"
#include "log_factory.h"

#include <iostream>
#include <string>
                    
#define DEFAULTKEY "1234567890abcdef1234567890abcdef"

using namespace std;

static int TimeToSeconds(const string& s)
{
    int hour = stoi(s.substr(0, 2));
    int minute = stoi(s.substr(3, 2));
    int seconds = stoi(s.substr(6, 2));
    return hour * 3600 + minute * 60 + seconds;
}

bool FFTask::Start(const Data& data)
{
    string cmd = "ffmpeg -y";
    if (data.task_type == "play")
    {
        cmd = "ffplay";
    }

    // 源视频
    cmd += " -i " + data.src;
    
    // 生成ffmpeg 命令并执行
    if (!data.begin_seconds.empty())
    {
        cmd += " -ss " + data.begin_seconds;
    }
    if (!data.end_seconds.empty())
    {
        int segment_seconds = stoi(data.end_seconds) - stoi(data.begin_seconds);
        if (segment_seconds > 0)
        {
            cmd += " -t " + to_string(segment_seconds);
        }
    }

    if (!data.secret_key.empty())
    {
        string default_key = DEFAULTKEY;

        // 保证密钥为32位
        for (int i = 0; i < data.secret_key.size() && i < default_key.size(); ++ i)
        {
            default_key[i] = data.secret_key[i];
        }

        // 加密命令
        if (data.is_encrypt)
        {
            cmd += " -encryption_scheme cenc-aes-ctr -encryption_kid abcdef0123456789abcdef0123456789";
            cmd += " -encryption_key ";
            cmd += default_key;
            cmd += " -c:v libx264 -c:a aac";
        }
        else    // 解密命令 
        {
            cmd += " -decryption_key ";
            cmd += default_key;
            cmd += " -c copy";
        }
        cout << "key size: " << default_key.size() << endl;
    }
    
    if (data.task_type == "cv")
    {
        cmd += " " + data.des;
    }
    
    cout << "FFTask Start: " << cmd << endl;

    // 需要访问对象内部的成员，需要传入this指针
    exec_.Start(cmd, [this](const string& s){
        // cout << s << endl;
        if (total_seconds_ <= 0)
        {
            auto pos = s.find("Duration: ");
            if (pos != string::npos)
            {
                string time = s.substr(pos + 10, 8);
                total_seconds_ = TimeToSeconds(time);
                return;  // 退出回调函数
            }
        }

        // 获取进度 (时间特征)

        auto pos = s.find("time=");
        if (pos != string::npos)
        {
            string time = s.substr(pos + 5, 8);
            int seconds = TimeToSeconds(time);
            if (total_seconds_ > 0)
                progress_ = 100 * seconds / total_seconds_;
            return;  // 退出回调函数
        }

    });

    // exec_.Stop();  // 等待子线程关闭 否则程序 Abort

    return true;
}

void FFTask::Stop()
{
    exec_.Stop();
}