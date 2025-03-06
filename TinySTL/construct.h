#ifndef MYTINYSTL_CONSTRUCT_H_
#define MYTINYSTL_CONSTRUCT_H_

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造
// destroy   : 负责对象的析构

// C++中 new 和 delete 负责内存的分配和释放
// 构造函数和析构函数负责对象的构造
// STL 将内存分配 allocator 和 对象生命周期管理 construct / destroy 结构

// 对不同参数类型重载构造和析构函数

#include <new>              // placement new

#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unused parameter
#endif // _MSC_VER

namespace mystl
{

// construct 构造函数
// 默认构造版本
template <typename Ty>
void construct(Ty *ptr)
{
    ::new((void*)ptr) Ty();     // 全局 placement new, 调用默认构造函数
}

// 拷贝构造函数
template <class Ty1, class Ty2>
void construct(Ty1* ptr, const Ty2& value)
{
  ::new ((void*)ptr) Ty1(value);    // 显式指定目标类型
}

// 完美转发 ?
template <typename Ty, class... Args>
void construct(Ty* ptr, Args&&... args)
{
    // 全局placement new, 使用::new 绕过特定的 operator new重载
    // void* 强制转换确保类型系统兼容
    ::new ((void*)ptr) Ty(mystl::forward<Args>(args)...);
}

// 析构对象
// 针对 trivially destructible 类型的空操作 
template <typename Ty>
void destroy_one(Ty*, std::true_type) { }

// 非 trivial 类型的析构造作
template <class Ty>
void destroy_one(Ty* pointer, std::false_type)
{
  if (pointer != nullptr)
  {
    pointer->~Ty();     // 显式调用析构函数
  }
}

// 范围析构的 trivial 版本
template <typename ForwardIter>
void destroy_cat(ForwardIter, ForwardIter, std::true_type) { }

// 范围析构的非 trivial 版本
template <class ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, std::false_type)
{
  for (; first != last; ++first)
    destroy(&*first);       // 获取元素地址，调用析构函数
}

// 单个对象析构入口
template <class Ty>
void destroy(Ty* pointer)
{
    // 通过类型特征选择实现版本
    destroy_one(pointer, std::is_trivially_destructible<Ty>{});
}

// 范围析构函数入口, 根据类型选择析构函数版本
template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last)
{
    using value_type = typename iterator_traits<ForwardIter>::value_type;
  destroy_cat(first, last, std::is_trivially_destructible<value_type>{});
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

}

#endif 