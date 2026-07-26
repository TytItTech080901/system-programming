#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define NUMLIST_LEN 1000 * 1000 * 50
#define NUM_THREADS 2

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

int numlist[NUMLIST_LEN] = { 0 };
int64_t g_sum = 0;

struct thread_info {
	int start;
	int end;
	pthread_t tid;
};

static pthread_mutex_t mutex;

static void *thread_start(void *arg)
{
	struct thread_info *tinfo = arg;
	int64_t local_sum = 0;

	for (int i = tinfo->start; i < tinfo->end; i++) {
		local_sum += numlist[i] * numlist[i] - numlist[i];
	}

	pthread_mutex_lock(&mutex);
	g_sum += local_sum;
	pthread_mutex_unlock(&mutex);

	printf("Thread %lu: processing indices [%d, %d)\n", tinfo->tid, tinfo->start, tinfo->end);

	return NULL;
}

static void single_thread_sum(void)
{
	int64_t sum = 0;
	for (int i = 0; i < NUMLIST_LEN; i++) {
		sum += numlist[i] * numlist[i] - numlist[i];
	}
	g_sum = sum;
}

int main(int argc, char *argv[])
{
	int use_multi_thread = 1;

	if (argc > 1) {
		if (strcmp(argv[1], "single") == 0) {
			use_multi_thread = 0;
		} else if (strcmp(argv[1], "multi") == 0) {
			use_multi_thread = 1;
		}
	}

	srand(42);
	for (int i = 0; i < NUMLIST_LEN; i++) {
		int rand_num = rand() % 100;
		numlist[i] = rand_num;
	}

	PROFILE_BEGIN();

	if (use_multi_thread) {
		printf("muti version: %d threads\n", NUM_THREADS);

		pthread_t tids[NUM_THREADS] = { 0 };
		struct thread_info tinfos[NUM_THREADS] = { 0 };
		int ret;
		void *tret;
		pthread_mutex_init(&mutex, NULL);

		int chunk_size = NUMLIST_LEN / NUM_THREADS;
		for (int i = 0; i < NUM_THREADS; i++) {
			tinfos[i].start = i * chunk_size;
			tinfos[i].end = (i == NUM_THREADS - 1) ? NUMLIST_LEN : (i + 1) * chunk_size;
			tinfos[i].tid = i;

			ret = pthread_create(&tids[i], NULL, thread_start, &tinfos[i]);
			if (ret) {
				fprintf(stderr, "Error: %s\n", strerror(ret));
				exit(-1);
			}
		}

		for (int i = 0; i < NUM_THREADS; i++) {
			ret = pthread_join(tids[i], &tret);
			if (ret) {
				fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
				exit(-1);
			}
		}
		pthread_mutex_destroy(&mutex);
	} else {
		printf("single version\n");
		single_thread_sum();
	}

	printf("the sum of numlist is %ld\n", g_sum);

	PROFILE_END(use_multi_thread ? "Multi-thread done" : "Single-thread done");

	return 0;
}

/* Jetbrain Mono */
