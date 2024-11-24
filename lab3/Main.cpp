#include <iostream>
#include <random>

#include <fstream>
#include <chrono>
#include <memory>
#include <string>

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
		"LOADS EACH THREAD WITH COMPUTATIONS AND WRITES RESULT TO FILE %s,"
		" WHICH REQUIRES THREAD SYNCHRONIZATION\n"
		"COMPUTATIONS: SQRT AND SQR OF EACH ARRAY MEMBER\n"
		"ARRAY SIZE: %llu | NUMBER OF COMPUTATIONS PER ARRAY[i]: %llu\n"
		"======================================\n",
		lab::FILE_NAME,
		lab::SIZE,
		lab::ITERATIONS);

	std::cout << "PRESS ANY KEY ...\n";
	std::cin.get();

	ThreadPool pool;
	size_t threadCount = pool.getThreadCount();

	//to stop console
	std::this_thread::sleep_for(std::chrono::seconds(3));

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
	printf("%u is doing computations\n", getThreadId());
	for (size_t i = bottom; i < top; ++i)
	{
		for (size_t j = 0; j < lab::ITERATIONS; ++j)
		{
			arr[i] = sqrt(arr[i]);
			arr[i] *= arr[i];
		}
	}
	printf("%u has done all computations\n", getThreadId());

	std::string buffer;
	buffer.reserve(top - bottom + 1);

	printf("%u is writing result to buffer\n", getThreadId());
	for (int i = bottom; i < top; ++i)
	{
		buffer += std::to_string(arr[i]) + '\n';
	}
	printf("%u has done writing to buffer\n", getThreadId());

	std::lock_guard<std::mutex> lock(*pMut);
	std::ofstream file(fileName, std::ios::app);
	printf("%u opened file\n", getThreadId());

	file.seekp(bottom);
	file << buffer;

	file.close();
	printf("%u closed file\n", getThreadId());
}
