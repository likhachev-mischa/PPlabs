#include <iostream>
#include <math.h>
#include <random>

#include "ThreadPool.h"
#include <fstream>
#include <chrono>

void job(float* arr, size_t bottom, size_t top, int id, std::mutex* mut, const char* fileName);
const size_t ITERATIONS = 100000;

int main()
{
	ThreadPool pool;
	size_t poolCount = pool.getThreadCount();
	std::cout << "Pool count: " << poolCount << '\n';

	const size_t SIZE = 100000;
	float* arr = new float[SIZE];

	srand(std::time(nullptr));
	for (size_t i = 0; i < SIZE; ++i)
	{
		arr[i] = rand() % SIZE;
	}

	const char* FILE_NAME = "Output.txt";
	std::ofstream file(FILE_NAME);
	file.clear();
	file.close();

	size_t step = static_cast<size_t>(floor(SIZE / poolCount));
	size_t currentStep = 0;
	std::mutex mut;
	for (size_t i = 0; i < poolCount; ++i)
	{
		size_t prevStep = currentStep;
		if ((currentStep + step) > (SIZE - step))
		{
			currentStep = SIZE;
		}
		else
		{
			currentStep += step;
		}
		pool.queueJob(std::bind(job, arr, prevStep, currentStep, i, &mut, FILE_NAME));
	}

	//wait for jobs to be passed to threads before joining them
	std::this_thread::sleep_for(std::chrono::seconds(2));

}

void job(float* arr, size_t bottom, size_t top, int id, std::mutex* mut, const char* fileName)
{
	for (size_t i = bottom; i < top; ++i)
	{
		for (size_t j = 0; j < ITERATIONS; ++j)
		{
			arr[i] = sqrt(arr[i]);
			arr[i] *= arr[i];
		}
	}

	std::unique_lock<std::mutex> fileMutex(*mut);
	std::ofstream file(fileName, std::ios::app);
	std::cout << "id " << id << " opened file\n";

	file.seekp(bottom);
	for (size_t i = bottom; i < top; ++i)
	{
		file << arr[i] << '\n';
	}

	file.close();
	std::cout << "id " << id << " closed file\n";
}
