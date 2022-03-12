#include "ThreadPool.h"

#include <conio.h>
#include <stdio.h>

typedef struct teststruct_t
{
	int index;
	int sum;
	int* numbers;
} teststruct;

void sumtest(void* arg)
{
	teststruct* ts = (teststruct*)arg;
	ts->sum += ts->numbers[ts->index++];
}

int main()
{
	int sumref = 0;
	int* numbers = (int*)calloc(64, sizeof(int));
	for (int i = 0; i < 64; i++)
	{
		numbers[i] = rand() % 100;
		sumref += numbers[i];
	}

	teststruct* ts = (teststruct*)calloc(1, sizeof(teststruct));
	ts->numbers = numbers;

	int threads = get_cpu_threads();

	thread_pool* threadpool = thread_pool_create(threads);

	for (int i = 0; i < 64; i++)
	{
		thread_pool_add_work(threadpool, sumtest, ts);
	}

	thread_pool_wait(threadpool);
	thread_pool_destroy(threadpool);

	printf("ref: %d out: %d\n", sumref, ts->sum);

	printf("Press any key to exit.");
	_getch();

	return 0;
}