#include <chrono>
#include <iostream>
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid()
#include <unistd.h>   // fock

typedef long long LL;

LL get_interval_sum(int start)
{
	start *= 20000;
	int stop = start + 20000;
	LL temp = 0;
	auto start_time = std::chrono::steady_clock::now();
	for (int i = start; i < stop; ++ i) {
		temp += i * (LL)i;
	}
	auto end_time = std::chrono::steady_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
	printf("I'm %dth, temp = %lld, time consumed: %ld\n", start / 20000, temp, duration_ms);
	return temp;
}

int main() 
{
	LL total_sum = 0;
	// 5个进程并行计算
	int THREAD_NUM = 5;
	auto start = std::chrono::steady_clock::now();
	int i;
	// 制造 5 个子进程 计算 [1, 100000] 的 1/5 个区间
	for (i = 0; i < THREAD_NUM; ++ i) {
		pid_t pid = fork();
		if (pid == 0) {
			// 进程之间变量不可见 应该如何通信?
			total_sum += get_interval_sum(i);
			break;  // 跳出循环 否则子进程会继续生成进程 造成指数级的冗余 2^n - 1
		}
	}

	if (i < THREAD_NUM) {  // 子进程 退出
		exit(0);		
	} else {  // 父进程
		int reclaim_num = 0;
		// 回收所有子进程
		do {
			// wait(NULL) 阻塞 回收子进程 NULL: 不关心子进程退出状态
			// waitpid 可以回收指定子进程
			//arg_1: pid, 指定子进程pid, -1表示任意子进程
			// arg_2: 指针，出参(通过指针将数值传出) 表示子进程退出状态 NULL表示不关心子进程状态
			//arg_3: WNOHANG 不阻塞，通过轮询回收，回收成功返回子进程pid, 失败返回0，出错返回-1
			auto wpid = waitpid(-1, NULL, WNOHANG);
			if (wpid > 0) {
				++ reclaim_num;
			}
		} while (reclaim_num != THREAD_NUM);

		printf("I'm parent, pid = %u, ppid = %u\n", getpid(), getppid());
		printf("linear scan result through multi-process is %lld\n", total_sum);
		auto end = std::chrono::steady_clock::now();
		auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		printf("linear scan through multi-process takes %ld ms\n", duration_ms);
	}

	return 0;
}
