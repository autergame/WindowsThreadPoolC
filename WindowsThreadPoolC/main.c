#include "ThreadPool.h"

#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

void start_time_func(LARGE_INTEGER* start_time, LARGE_INTEGER* frequency)
{
	QueryPerformanceFrequency(frequency);
	QueryPerformanceCounter(start_time);
}

double stop_time_func(LARGE_INTEGER start_time, LARGE_INTEGER frequency)
{
	LARGE_INTEGER end_time;
	QueryPerformanceCounter(&end_time);
	return ((double)((end_time.QuadPart - start_time.QuadPart) * 1000000) / (double)frequency.QuadPart) / 1000.0;
}

typedef struct teststruct_t
{
	int index;	
	uint64_t* sum;
	uint64_t* numbers;
} teststruct;

void sumtest(void* arg)
{
	teststruct* ts = (teststruct*)arg;
	*ts->sum += (ts->numbers[ts->index] * ts->numbers[ts->index]) / 3;
	printf("%04d ", ts->index);
	free(ts);
}

int main()
{
	LARGE_INTEGER start_time, frequency;

	int number_size = 1000;
	uint64_t* numbers = (uint64_t*)calloc(number_size, sizeof(uint64_t));
	for (int i = 0; i < number_size; i++)
	{
		numbers[i] = rand() % 100;
	}

	{
		uint64_t sum_ref = 0;
		start_time_func(&start_time, &frequency);
		for (int i = 0; i < number_size; i++)
		{
			sum_ref += (numbers[i] * numbers[i]) / 3;
			printf("%04d ", i);
		}
		printf("end\n");
		double sum_elapsed_ref = stop_time_func(start_time, frequency);

		printf("ref: %" PRIu64 "\n", sum_ref);
		printf("ref total time: %9.6f ms\n", sum_elapsed_ref);
	}

	{
		int cpu_threads = get_cpu_threads();
		thread_pool* thread_pool = thread_pool_create(cpu_threads);

		uint64_t sum_out = 0;
		start_time_func(&start_time, &frequency);
		for (int i = 0; i < number_size; i++)
		{
			teststruct* ts = (teststruct*)calloc(1, sizeof(teststruct));
			ts->numbers = numbers;
			ts->sum = &sum_out;
			ts->index = i;
			thread_pool_add_work(thread_pool, sumtest, ts);
		}
		thread_pool_wait(thread_pool);
		printf("end\n");
		double sum_elapsed = stop_time_func(start_time, frequency);

		thread_pool_destroy(thread_pool);

		printf("out: %" PRIu64 "\n", sum_out);
		printf("out total time: %9.6f ms\n", sum_elapsed);
	}

	free(numbers);

	printf("Press any key to exit.");
	_getch();

	return 0;
}