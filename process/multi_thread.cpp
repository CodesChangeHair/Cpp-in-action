#include <iostream>
#include <thread>

void hello() 
{
	std::cout << "hello world" << std::endl;
}

void print(const std::string& name)
{
	std::cout << name << std::endl;
}

void test()
{
	// 创建一个 thread 对象，并指定入口函数
	std::thread t(hello);
	t.join();
	std::thread t2([] {
		std::cout << "hello lambda" << std::endl;
	});
	t2.join();
	std::thread t3(print, "transfer parameter");  // 传递参数
	t3.join();
}

int main()
{
	test();
	return 0;
}
