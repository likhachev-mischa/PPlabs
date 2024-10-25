#include <iostream>
#include <math.h>
#include <random>

#include "ThreadPool.h"
#include <fstream>


void job(float* arr, size_t bottom, size_t top)
{
	//using namespace std::chrono_literals;
	//std::cout << "job start\n";
	//for (int i = 0; i < 10; ++i)
	//{

	//	//printf("job thread %llu ,value %i\n", std::hash<std::thread::id>{}(std::this_thread::get_id()), value);
	////	std::cout << "job thread "<<std::this_thread::get_id()<<'\t'<<value << '\n';
	//	std::this_thread::sleep_for(1000ms);
	//}
	for (size_t i = bottom; i < top; ++i)
	{
		for (size_t j = 0; j < 100000; ++j)
			arr[i] = sqrt(arr[i]);
	}

	std::unique_lock<std::mutex> (fileMutex);
	std::ofstream file("Output.txt", std::ios::app);
	file.seekp(bottom);
	for (size_t i = bottom; i < top; ++i)
	{
		file << arr[i] << '\n';
	}
	file.close();
}

int main()
{
	ThreadPool pool;
	pool.start();
	size_t poolCount = pool.getThreadCount();
	std::cout << "Pool count: " << poolCount << '\n';
	const size_t SIZE = 100000;
	float arr[SIZE];

	srand(std::time(nullptr));
	for (size_t i = 0; i < SIZE; ++i)
	{
		arr[i] = rand() % 100000;
	}


	size_t step = SIZE / poolCount - 1;
	for (size_t i = 0; i < poolCount; ++i)
	{
		pool.queueJob(std::bind(job, arr, i * step, (i + 1) * step));
	}

	pool.stop();
}
