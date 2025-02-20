#include <iostream>
#include <chrono>

typedef long long LL;

int main() 
{
	auto start = std::chrono::steady_clock::now();
	LL new_total = 0;
	for (int i = 0; i < 100000; ++ i) {
		new_total += i * (LL)i;
	}
	printf("linear scan result = %lld\n", new_total);
	auto end = std::chrono::steady_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	printf("linear scan takes %lld ms\n", duration_ms);
	return 0;
}
