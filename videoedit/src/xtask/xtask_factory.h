#pragma once

/// 创建 Task 对象


// 只需要提供虚基函数声明作为接口，用户不需要知道具体的实现
// 具体实现(利用多态机制) 防止在独立编译单元cpp文件中
#include "xtask.h"

#include <memory>

class XTaskFactory
{
public:
    static std::unique_ptr<XTask> Create(int type = 0);
};