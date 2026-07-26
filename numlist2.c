#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

typedef struct Task {
	int *arr;
	int start;
	int end;
	long long result;
	struct Task *next;
} Task;

typedef struct ThreadPool {
	pthread_t *threads;
	int thread_count;
	Task *task_queue_head;
	Task *task_queue_tail;
	int queue_size;
	int shutdown;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_cond_t done_cond;
	int active_task;
} ThreadPool;

#define NUMLIST_LEN 1000 * 1000 * 50
#define NUM_THREADS 5
#define NUM_TASK 40

ThreadPool *pool = NULL;

int numlist[NUMLIST_LEN];

static double get_secs(void)
{
	struct timespec ts = { 0 };
	int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (ret) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

double begin;
#if 1
#define PROFILE_BEGIN() begin = get_secs();
#define PROFILE_END(label) printf("%s: %lfsecs\n", (label), get_secs() - begin);
#else
#define PROFILE_BEGIN(...)
#define PROFILE_END(...)
#endif

static void *thread_start(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;

	for (;;) {
		pthread_mutex_lock(&pool->mutex);

		while (pool->task_queue_head == NULL && !pool->shutdown) {
			pthread_cond_wait(&pool->cond, &pool->mutex);
		}

		if (pool->shutdown && pool->task_queue_head == NULL) {
			pthread_mutex_unlock(&pool->mutex);
			break;
		}

		Task *task = pool->task_queue_head;
		if (task != NULL) {
			pool->task_queue_head = task->next;
			if (pool->task_queue_head == NULL) {
				pool->task_queue_tail = NULL;
			}
			pool->queue_size--;
			pool->active_task++;
		}

		pthread_mutex_unlock(&pool->mutex);

		if (task != NULL) {
			long long sum = 0;
			for (int i = task->start; i < task->end; i++) {
				sum += task->arr[i];
			}
			task->result = sum;
			printf("task: processing indices [%d, %d)\n", task->start, task->end);
		}

		pthread_mutex_lock(&pool->mutex);
		pool->active_task--;
		if (pool->active_task == 0 && pool->task_queue_head == NULL) {
			pthread_cond_signal(&pool->done_cond);
		}
		pthread_mutex_unlock(&pool->mutex);
	}

	return NULL;
}

ThreadPool *create_thread_pool(int thread_count)
{
	ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
	pool->thread_count = thread_count;
	pool->task_queue_head = NULL;
	pool->task_queue_tail = NULL;
	pool->queue_size = 0;
	pool->shutdown = 0;
	pool->active_task = 0;

	pthread_mutex_init(&pool->mutex, NULL);
	pthread_cond_init(&pool->cond, NULL);
	pthread_cond_init(&pool->done_cond, NULL);

	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
	for (int i = 0; i < thread_count; i++) {
		pthread_create(&pool->threads[i], NULL, thread_start, pool);
	}

	return pool;
}

void add_task(ThreadPool *pool, Task *task)
{
	pthread_mutex_lock(&pool->mutex);

	task->next = NULL;
	if (pool->task_queue_tail == NULL) {
		pool->task_queue_head = task;
		pool->task_queue_tail = task;
	} else {
		pool->task_queue_tail->next = task;
		pool->task_queue_tail = task;
	}

	pool->queue_size++;

	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
}

void wait_all_tasks(ThreadPool *pool)
{
	pthread_mutex_lock(&pool->mutex);
	while (pool->active_task > 0 || pool->task_queue_head != NULL) {
		pthread_cond_wait(&pool->done_cond, &pool->mutex);
	}
	pthread_mutex_unlock(&pool->mutex);
}

void destroy_thread_pool(ThreadPool *pool)
{
	pthread_mutex_lock(&pool->mutex);
	pool->shutdown = 1;
	pthread_cond_broadcast(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);

	for (int i = 0; i < pool->thread_count; i++) {
		pthread_join(pool->threads[i], NULL);
	}

	free(pool->threads);
	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
	pthread_cond_destroy(&pool->done_cond);
	free(pool);
}

int main()
{
	srand(666);
	for (int i = 0; i < NUMLIST_LEN; i++) {
		numlist[i] = rand() % 100;
	}

	PROFILE_BEGIN();

	pool = create_thread_pool(NUM_THREADS);

	int chunk_size = NUMLIST_LEN / NUM_TASK;
	Task *tasks = (Task *)malloc(sizeof(Task) * NUM_TASK);

	for (int i = 0; i < NUM_TASK; i++) {
		tasks[i].arr = numlist;
		tasks[i].start = i * chunk_size;
		tasks[i].end = (i == NUM_TASK - 1) ? NUMLIST_LEN : (i + 1) * chunk_size;
		tasks[i].result = 0;
		tasks[i].next = NULL;
		add_task(pool, &tasks[i]);
	}

	wait_all_tasks(pool);

	long long total = 0;
	for (int i = 0; i < NUM_TASK; i++) {
		total += tasks[i].result;
	}

	printf("the sum of numlist is %lld\n", total);

	PROFILE_END("thread pool done");

	free(tasks);
	destroy_thread_pool(pool);

	return 0;
}
