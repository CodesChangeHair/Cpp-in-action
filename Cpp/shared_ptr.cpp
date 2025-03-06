#include <iostream>
#include <atomic>   // 用于原子操作

// SharedPtr C++11引入的智能指针，多个指针可以共享一个对象。智能指针
// 内部包含两个指针，一个是托管的指向资源的原始指针，一个是指向控制块的指针
// 控制块用于引用计数，当调用构造函数或拷贝操作时，引用计数+1，当
// 智能指针引用新对象或析构时，原资源的引用计数-1。当引用计数为0时，销毁资源.

// noexcept: C++11引入的关键字，用于声明一个函数不会抛出异常, 有两个核心作用
// 1. 优化机会: 编译期知道函数不会抛出异常后，可以省略异常处理机制(如栈展开),
//      生成更高效的代码
// 2. 契约保证: 强制要求函数实现不抛出异常，否则程序会直接调用 std::terminate()
//      终止，避免异常传播
// 应用场景:
// 移动构造函数 / 移动赋值运算符 在重新分配内存时，如果元素的移动操作标记为
// noexcept, 则会优先使用移动而非拷贝，提升性能.
// 析构函数: 析构函数默认不抛出异常，否则可能导致资源泄露.

// C++ 异常处理机制流程: 抛出-捕获 模型
// 1. 抛出异常: throw
// 2. 栈展开: Stack Unwinding, 程序终止当前执行流，沿着调用栈向上回溯，
// 寻找匹配的 catch 块. 回溯过程中，局部对象会被析构，确保资源释放(RAII)
// 3. 异常捕获 catch : try 块后的 catch 块捕获异常
// 4. 未捕获异常处理: 调用 std::terminate() 终止程序. 

// explicit: 用于修饰构造函数，或类型转换函数，防止隐式类型转换.
// 应用场景: 单个参数的构造函数; 转换运算符.

// 拷贝赋值运算符为何会返回对象引用?
// 为支持链式赋值操作，如 a = b = c. 这一设计遵循以下原则:
// 1. 与内置类型一致，如int
// 2. 链式操作
// 3. 避免不必要的拷贝

// delete ptr 的效果 以及 ptr 值的变化
// 效果:
//      释放内存，释放 ptr 指向的内存
//      调用析构函数: 如果ptr指向的是一个对象，则会调用该对象的析构函数
//              (先调用析构函数释放对象管理的资源，再释放对象占用的内存)
// delete ptr后，ptr的值不变，此时ptr成为一个悬空指针Dangling ptr,
// 继续访问或delete会导致未定义行为.

// 引用计数器，线程安全
class SharedCount
{
public:
    SharedCount() : count(1) { }

    // 增加计数，注意原子操作需要指定内存顺序
    void add() { count.fetch_add(1, std::memory_order_relaxed); }

    // 减少计数并返回当前值
    int minus() { return count.fetch_sub(1, std::memory_order_acq_rel) - 1; }

    int get() const { return count.load(std::memory_order_relaxed); }
private:
    std::atomic<int> count;  
};

template <typename T>
class SharedPtr 
{
public:
    SharedPtr() noexcept : ptr(nullptr), ref_count(nullptr) {}

    // 构造函数 确保 ptr 不为空是分配控制块
    explicit SharedPtr(T* ptr) : ptr(ptr_), ref_count(nullptr) 
    {
        if (ptr)
            ref_count = new SharedCount();
    }

    ~SharedPtr() { clean(); }

    // 拷贝构造函数
    SharedPtr(const SharedPtr& other) noexcept
        : ptr(other.ptr), ref_count(other.ref_count)
    {
        if (ref_count)
            ref_count->add();
    }

    // 移动构造函数，转移所有权
    SharedPtr(SharedPtr&& other) noexcept
        : ptr(other.ptr), ref_count(other.ref_count)
    {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }

    // 拷贝赋值运算符 考虑自身赋值和异常安全
    SharedPtr& operator=(const SharedPtr& other) noexcept
    {
        if (this != &other)
        {
            clean();    // 先释放原有资源

            ptr = other.ptr;
            ref_count = other.ref_count;

            if (ref_count)
                ref_count->add();
        }
        return *this;
    }

    // 移动赋值运算符
    SharedPtr& operator=(SharedPtr&& other) noexcept 
    {
        if (this != &other)
        {
            clean();  // 释放原有资源

            ptr = other.ptr;
            ref_count = other.ref_count;

            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }

private:
    T* ptr;     // 托管的原始指针
    SharedCount* ref_count;  // 控制块指针

    void clean() noexcept
    {
        if (ref_count)
        {
            if (ref_count->minus() == 0) {
                delete ptr;         // 释放资源
                delete ref_count;   // 释放控制块
            }
            ptr = nullptr;
            ref_count = nullptr;
        }
    }

public:
    void reset(T* new_ptr = nullptr) noexcept 
    {
        clean();
        if (new_ptr) {
            ptr = new_ptr;
            ref_count = new SharedCount();
        }
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(ref_count, other.ref_count);
    }
};

// non-member function swap
template <typename T>
void swap(SharedPtr<T>& a, SharedPtr<T>& b) noexcept 
{
    a.swap(b);
}