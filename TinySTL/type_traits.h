#ifndef MYTINYSTL_TYPE_TRAITS_H_
#define MYTINYSTL_TYPE_TRAITS_H_

// 这个头文件用于提取类型信息
// 类型萃取是泛型编程的核心工具，用于在编译期获取类型特性

#include <type_traits>  // 标准库类型萃取

namespace mystl 
{
// helper struct
// 实现编译期常量包装器，类似 std::integral_constant
// 用于创建携带类型信息的编译期常量

template <typename T, T v>
struct m_integral_constant 
{
    static constexpr T value = v;   // 存储编译期常量
};

// 模板别名简化布尔常量创建
template <bool b>
using m_bool_constant = m_integral_constant<bool, b>;

// 预定义布尔常量类型
// struct m_integral_const 
// {
//     static constexpr bool value = true / false;
// };
typedef m_bool_constant<true>  m_true_type;
typedef m_bool_constant<false> m_false_type;

// *********************************
// type traits

// is pair
// 用于检测类型是否为pair, 常见于 map 的 value_type 处理

// forward declaration begin
template <typename T1, typename T2>
struct pair;
// forward declaration end

// 主模板默认返回 false 
template <typename T>
struct is_pair : mystl::m_false_type {};    // 继承 false 类型

// 特化版本检测 pair 类型
template <typename T1, typename T2>
struct is_pair<mystl::pair<T1, T2>> : m_true_type {};  // 继承 true 类型

}

#endif 