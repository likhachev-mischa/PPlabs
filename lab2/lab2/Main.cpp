#include <iostream>

#include "ThreadPool.h"


void job()
{
	using namespace std::chrono_literals;
	std::cout << "job start\n";
	for (int i = 0; i < 10; ++i)
	{
		std::cout << "job thread "<<std::this_thread::get_id()<<'\n';
		std::this_thread::sleep_for(1000ms);
	}
	std::cout << "job finish\n";
}

int main()
{
	ThreadPool pool;
	pool.start();
	std::cout << "Pool count: " << pool.getThreadCount() << '\n';
	pool.queueJob(job);

	using namespace std::chrono_literals;
	for (int i = 0; i < 5; ++i)
	{
		std::cout << "main thread "<< std::this_thread::get_id()<<'\n';
		std::this_thread::sleep_for(1000ms);
	}

	pool.stop();
}
