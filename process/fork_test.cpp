#include <unistd.h>
#include <iostream>

int main()
{
	int x = 0;
	std::cout << "x = " << x << std::endl;
	std::cout << "parent pid = " << getpid() << std::endl;
	auto pid = fork();
	if (pid == 0) {
		++ x;
		std::cout << "x = " << x << std::endl;
		std::cout << "success" << std::endl;
		std::cout << "child getpid = " << getpid() << std::endl;		
	} else if (pid < 0) {
		std::cout << "failure" << std::endl;
	} else {
		std::cout << "x = " << x << std::endl;
		std::cout << "pid = " << pid << std::endl;
	}
	return 0;
}
