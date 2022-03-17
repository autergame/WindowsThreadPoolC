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

typedef struct test_struct_
{
	int index;
	uint64_t sum;
	uint64_t* numbers;
} test_struct;

thread_pool_function(sumtest, arg_var)
{
	test_struct* ts = (test_struct*)arg_var;

	DWORD indexlock = InterlockedIncrement(&ts->index);
	int index = indexlock - 1;

	uint64_t calc = (ts->numbers[index] * ts->numbers[index]) / 3;

	InterlockedAdd64(&ts->sum, calc);

	//DWORD thread_id = GetCurrentThreadId();
	//printf("thread: %06d index: %03d calc: %04" PRIu64 "\n", thread_id, index, calc);
}

int main()
{
	LARGE_INTEGER start_time, frequency;

	int number_size = 100000;
	uint64_t* numbers = (uint64_t*)calloc(number_size, sizeof(uint64_t));
	for (int i = 0; i < number_size; i++)
	{
		numbers[i] = (rand() % 100) + 5;
	}

	uint64_t sum_ref = 0;

	//printf("start\n");
	start_time_func(&start_time, &frequency);

	for (int i = 0; i < number_size; i++)
	{
		uint64_t calc = (numbers[i] * numbers[i]) / 3ull;
		sum_ref += calc;

		//printf("index: %03d calc: %04" PRIu64 "\n", i, calc);
	}

	double sum_elapsed_ref = stop_time_func(start_time, frequency);
	//printf("end\n");

	printf("	ref: %" PRIu64 "\n", sum_ref);
	printf("	ref total time: %9.6f ms\n\n", sum_elapsed_ref);

	int total_tests = 100;
	int total_errors = 0;
	for (int k = 0; k < total_tests; k++)
	{
		printf("test number: %d\n", k + 1);

		test_struct* ts = (test_struct*)calloc(1, sizeof(test_struct));
		ts->numbers = numbers;
		ts->index = 0;
		ts->sum = 0;

		//printf("start\n");
		start_time_func(&start_time, &frequency);

		int cpu_threads = get_cpu_threads();

		thread_pool* thread_pool = thread_pool_create(cpu_threads);

		for (int i = 0; i < number_size; i++)
		{
			thread_pool_add_work(thread_pool, sumtest, ts);
		}

		thread_pool_destroy(thread_pool);

		double sum_elapsed = stop_time_func(start_time, frequency);
		//printf("end\n");

		printf("	out: %" PRIu64 "\n", ts->sum);
		printf("	out total time: %9.6f ms\n\n", sum_elapsed);

		if (sum_ref - ts->sum != 0)
			total_errors++;

		free(ts);
	}

	int percentage = 0;
	if (total_errors == 0)
		percentage = 100;
	else
		percentage = (int)(100.f - (100.f * ((float)total_errors / (float)total_tests)));
	printf("total errors: %d of %d -> %d%% correct\n\n", total_errors, total_tests, percentage);

	free(numbers);

	printf("Press any key to exit.");
	_getch();

	return 0;
}