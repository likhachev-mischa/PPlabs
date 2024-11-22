#include <iostream>
#include <random>

#include <fstream>
#include <chrono>
#include <memory>
#include "ThreadPool.hpp"
#include "Constants.hpp"

void job(std::shared_ptr<float[]> arr,
         size_t bottom,
         size_t top,
         std::shared_ptr<std::mutex> pMut,
         const char* fileName);

int main()
{
	printf(
		"THIS PROGRAM CREATES N THREADS (BASED ON HARDWARE STATS) \n"
		"LOADS EACH THREAD WITH COMPUTATIONS AND WRITES RESULT TO FILE Output.txt, WHICH REQUIRES THREAD SYNCHRONIZATION\n"
		"===================\n===================\n");

	ThreadPool pool;
	size_t threadCount = pool.getThreadCount();

	std::shared_ptr<float[]> arr(new float[lab::SIZE], std::default_delete<float[]>());

	srand(std::time(nullptr));
	for (size_t i = 0; i < lab::SIZE; ++i)
	{
		arr[i] = static_cast<float>(rand() % lab::SIZE);
	}

	std::ofstream file(lab::FILE_NAME);
	file.clear();
	file.close();

	size_t step = static_cast<size_t>(floor(lab::SIZE / threadCount));
	size_t currentStep = 0;
	std::shared_ptr<std::mutex> pMutex(new std::mutex);
	for (size_t i = 0; i < threadCount; ++i)
	{
		size_t prevStep = currentStep;
		if ((currentStep + step) > (lab::SIZE - step))
		{
			currentStep = lab::SIZE;
		}
		else
		{
			currentStep += step;
		}
		pool.queueJob(std::bind(job, arr, prevStep, currentStep, pMutex, lab::FILE_NAME));
	}
}

void job(std::shared_ptr<float[]> arr,
         size_t bottom,
         size_t top,
         std::shared_ptr<std::mutex> pMut,
         const char* fileName)
{
	printf("%lu is doing arbitrary math\n", GetCurrentThreadId());
	for (size_t i = bottom; i < top; ++i)
	{
		for (size_t j = 0; j < lab::ITERATIONS; ++j)
		{
			arr[i] = sqrt(arr[i]);
			arr[i] *= arr[i];
		}
	}
	printf("%lu has done all calculations\n", GetCurrentThreadId());

	std::lock_guard<std::mutex> lock(*pMut);
	std::ofstream file(fileName, std::ios::app);
	printf("%lu opened file\n", GetCurrentThreadId());

	file.seekp(bottom);
	for (size_t i = bottom; i < top; ++i)
	{
		file << arr[i] << '\n';
	}

	file.close();
	printf("%lu closed file\n", GetCurrentThreadId());
}
