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

typedef struct teststruct_array_t
{
	uint64_t sum;
	uint64_t* numbers;
} teststruct_array;

typedef struct teststruct_t
{
	int index;	
	teststruct_array* tsa;
} teststruct;

void sumtest(void* arg)
{
	teststruct* ts = (teststruct*)arg;
	teststruct_array* tsa = (teststruct_array*)ts->tsa;
	tsa->sum += (tsa->numbers[ts->index] * tsa->numbers[ts->index]) / 3;
	//printf("%04d ", ts->index);
	free(ts);
}

int main()
{
	LARGE_INTEGER start_time, frequency;

	int number_size = 10000;
	uint64_t* numbers = (uint64_t*)calloc(number_size, sizeof(uint64_t));
	for (int i = 0; i < number_size; i++)
	{
		numbers[i] = (rand() % 100) + 1;
	}

	int total_errors = 0;
	for (int k = 0; k < 50; k++)
	{
		printf("test number: %d\n", k);

		uint64_t sum_ref = 0;
		start_time_func(&start_time, &frequency);

		for (int i = 0; i < number_size; i++)
		{
			sum_ref += (numbers[i] * numbers[i]) / 3ull;
			//printf("%04d ", i);
		}

		double sum_elapsed_ref = stop_time_func(start_time, frequency);
		//printf("end\n");

		printf("	ref: %" PRIu64 "\n", sum_ref);
		printf("	ref total time: %9.6f ms\n", sum_elapsed_ref);

		teststruct_array* tsa = (teststruct_array*)calloc(1, sizeof(teststruct_array));
		tsa->numbers = numbers;

		start_time_func(&start_time, &frequency);

		int cpu_threads = get_cpu_threads();
		thread_pool* thread_pool = thread_pool_create(cpu_threads);

		for (int i = 0; i < number_size; i++)
		{
			teststruct* ts = (teststruct*)calloc(1, sizeof(teststruct));
			ts->tsa = tsa;
			ts->index = i;
			thread_pool_add_work(thread_pool, sumtest, ts);
		}

		thread_pool_wait(thread_pool);
		thread_pool_destroy(thread_pool);

		double sum_elapsed = stop_time_func(start_time, frequency);
		//printf("end\n");

		printf("	out: %" PRIu64 "\n", tsa->sum);
		printf("	out total time: %9.6f ms\n\n", sum_elapsed);

		if (sum_ref - tsa->sum != 0)
			total_errors++;

		free(tsa);
	}

	printf("total errors: %d\n", total_errors);

	free(numbers);

	printf("Press any key to exit.");
	_getch();

	return 0;
}