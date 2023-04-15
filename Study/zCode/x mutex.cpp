#include <iostream>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <chrono>
std::mutex m;

void proc1(int& a) {
	static int counter = 100;
	while (true&&counter>0)
	{
		counter -= 1;
		bool isMode = counter % 10 == 0;
		m.lock();
		if(isMode) std::cout << "proc1 函数正在改写 a" << std::endl;
		if(isMode) std::cout << "原始 a为" << a << std::endl;
		a = a + 2;
		if(isMode) std::cout << "现在 a为" << a  << std::endl;
		m.unlock();
		auto time = std::chrono::nanoseconds(330);
		std::this_thread::sleep_for(time);
	}
}
void proc2(int& a) {
	static int counter = 1000;
	while (true&&counter>0)
	{
		counter -= 1;
		bool isMode = counter % 100 == 0;
		m.lock();
		if(isMode) std::cout << "proc2 函数正在改写 a" << std::endl;
		if(isMode) std::cout << "原始 a为" << a << std::endl;
		a = a + 1;
		if(isMode) std::cout << "现在 a为" << a << std::endl;
		m.unlock();
		auto time = std::chrono::nanoseconds(100);
		std::this_thread::sleep_for(time);
	}
}
int mainx() 
{
	int a = 0;
	std::thread t1(proc1,std::ref(a));
	std::thread t2(proc2,std::ref(a));
	int counter = 10000;
	while (counter>0)
	{
		counter -= 1;
		if (counter % 100 == 0) 
		{
			m.lock();
			a = a + 3;
			std::cout << "主线程 a是" << a << std::endl;
			m.unlock();
		}
	}
	t1.join();
	t2.join();
	return 0;
}