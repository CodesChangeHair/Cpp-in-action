#pragma once

#include "xtask.h"
#include "xexec.h"  // 执行命令并获取命令返回内容

/// 基于 ffmpeg 和 ffplay 工具实现的音视频模块

class FFTask: public XTask
{
public:
    /// 开启音视频处理任务, 立刻返回，在线程中处理
    /// @param Data 任务配置参数
    /// return 任务是否正常执行
    bool Start(const Data&) override;
    
    /// 获取任务进度，进度值 0 ~ 100
    int Progress() override { return progress_; }

    /// 处理视频的总时长(s)
    /// 用于展示处理进度 当前处理视频时长 / 总时长
    int TotalSeconds() override { return total_seconds_; }

    bool Running() override { return exec_.Running(); }

    void Stop() override;  // 等待 exec_ 开启的线程结束

private:
    int total_seconds_{ 0 };  // 处理的视频的总时长
    int progress_{ 0 };
    XExec exec_;
};