#pragma once 

#include <string>

/// 音视频任务处理接口

class XTask
{
public:
    // 任务需要的数据
    struct Data
    {
        std::string task_type;  // 任务类型 cv play
        std::string src;        // 源文件
        std::string des;        // 目标文件
        std::string begin_seconds;  // 剪切开始时间
        std::string end_seconds;    // 剪切结束时间
        std::string secret_key;     // 加密/解密密钥
        bool is_encrypt{ true };       // 是否加密 false表示当前需要解密
    };

    /// 开启音视频处理任务, 立刻返回，在线程中处理
    /// @param Data 任务配置参数
    /// return 任务是否正常执行
    virtual bool Start(const Data&) = 0;
    
    /// 获取任务进度，进度值 0 ~ 100
    virtual int Progress() = 0;

    /// 处理视频的总时长(s)
    /// 用于展示处理进度 当前处理视频时长 / 总时长
    virtual int TotalSeconds() = 0;

    virtual bool Running() = 0;

    virtual void Stop() = 0;  // 等待 exec_ 开启的线程结束

    ~XTask() {}
};