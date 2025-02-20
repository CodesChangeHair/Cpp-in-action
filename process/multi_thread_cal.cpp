#include <iostream>
#include <cmath>
#include <mutex>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <unistd.h>
#include <iomanip>

static const int MAX = 1e8;

using LL = long long;

static LL sum = 0;

void worker(int minV, int maxV) 
{
	for (int i = minV; i <= maxV; ++ i)
		sum += i * (LL)i;
}

void serial_task()
{
	// 单线程 计算1 ~ 1e8 的平方之和
	auto start = std::chrono::steady_clock::now();
	sum = 0;
	worker(1, MAX);
	auto end = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Serial task finish, " << ms << " ms consumed, Result: " << sum << std::endl;
}

static std::mutex mtx;

void concurrent_worker(int minV, int maxV)
{
	LL tmp = 0;
	for (int i = minV; i <= maxV; ++ i)
		tmp += i * (LL)i;

	mtx.lock();
	sum += tmp;
	mtx.unlock();
}

void concurrent_task(int minV, int maxV)
{
	auto start = std::chrono::steady_clock::now();
	// 获取当前硬件支持多少个线程并发执行
	unsigned concurrent_count = std::thread::hardware_concurrency();
	std::cout << " hardware_concurrency: " << concurrent_count << std::endl;
	
	std::vector<std::thread> threads;
	sum = 0;
	minV = 0;
	int range = maxV / concurrent_count;
	for (int t = 0; t < concurrent_count; ++ t)
	{
		if ( t < concurrent_count - 1) {
			threads.push_back(std::thread(concurrent_worker, minV, minV + range));
			minV += range + 1;
		} else {
			threads.push_back(std::thread(concurrent_worker, minV, maxV));
		}
	}
	
	for (auto& thread : threads)
		thread.join();

	auto end = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "concurent task finish, " << ms << " ms consumed, Result: " << sum << std::endl;
}

int main()
{
	serial_task();

	concurrent_task(1, MAX);

	return 0;
}
