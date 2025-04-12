#include <iostream>
#include <thread>
#include <shared_mutex> // C++17 提供的读写锁
#include <vector>
#include <chrono>
#include <mutex>

using namespace std;

shared_mutex rw_mutex; // 读写锁
int shared_data = 0;   // 共享资源

// 读者线程
void reader(int id)
{
    while (true)
    {
        shared_lock<shared_mutex> read_lock(rw_mutex);  // 获得读锁，多个读者可以并行
        cout << "[Reader " << id << "] 读取 shared_data: " << shared_data << endl;
        this_thread::sleep_for(chrono::milliseconds(100)); // 模拟读取过程
    }
}

// 写者线程函数
void writer(int id) 
{
    while (true) {
        cout << ">>> [Writer " << id << "] 尝试获取写锁..." << endl;
        unique_lock<shared_mutex> write_lock(rw_mutex); // 获得写锁（独占）
        cout << ">>> [Writer " << id << "] 成功获取写锁！" << endl;
        shared_data += 1;
        cout << ">>> [Writer " << id << "] 写入 shared_data 为: " << shared_data << endl;
        this_thread::sleep_for(chrono::milliseconds(200)); // 模拟写入过程
    }
}

int main()
{
    vector<thread> readers;
    vector<thread> writers;

    // 启动3个读线程
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back(reader, i);
    }

    // 启动2个写线程
    for (int i = 0; i < 2; ++i) {
        writers.emplace_back(writer, i);
    }

    // 等待线程结束（本例中不会结束）
    for (auto& t : writers) t.join();
    for (auto& t : readers) t.join();

    return 0;
}