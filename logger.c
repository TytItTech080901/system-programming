#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include "logger.h"

typedef struct log_item {
	struct timespec ts;
	log_event_t event;
	char module[32];
	char msg[256];
} log_item_t;

#define MAX_LOG_ENTRIES 1000

typedef struct {
	log_item_t log_items[MAX_LOG_ENTRIES];
	int head;
	int tail;
	int count;
	int max_size;
	int shutdown;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t worker;

	FILE *file_fp;
	FILE *extra_fp;
	log_event_t min_level;
} log_t;

static log_t g_log = { 0 };

static void get_time_str(char *buf, size_t size, const struct timespec *ts)
{
	struct tm tm;
	localtime_r(&ts->tv_sec, &tm);
	strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm);
}

static void *thread_start(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_log.mutex);

	for (;;) {
		/* 生产者没关，但是队列空了，等待*/
		while (g_log.count == 0 && !g_log.shutdown) {
			pthread_cond_wait(&g_log.cond, &g_log.mutex);
		}

		/* 生产者关了，队列空了，直接关闭*/
		if (g_log.count == 0 && g_log.shutdown) {
			pthread_mutex_unlock(&g_log.mutex);
			break;
		}

		log_item_t log_item = g_log.log_items[g_log.head];
		g_log.head = (g_log.head + 1) % MAX_LOG_ENTRIES;
		g_log.count--;
		pthread_mutex_unlock(&g_log.mutex);

		// 级别过滤（也可在生产者端做以节省队列空间，但这里做简单）
		if (log_item.event <= g_log.min_level) {
			char time_str[32];
			const char *event_str[] = { "ERROR", "WARN", "INFO", "DEBUG" };
			get_time_str(time_str, sizeof(time_str), &log_item.ts);

			if (g_log.file_fp) {
				fprintf(g_log.file_fp, "[%s] [%s] [%s] %s\n", time_str, event_str[log_item.event],
						log_item.module, log_item.msg);
				fflush(g_log.file_fp);
			}
			if (g_log.extra_fp) {
				fprintf(g_log.extra_fp, "[%s] [%s] [%s] %s\n", time_str, event_str[log_item.event],
						log_item.module, log_item.msg);
				fflush(g_log.extra_fp);
			}
		}

		pthread_mutex_unlock(&g_log.mutex);
	}

	return NULL;
}

void log_init(const char *log_file, log_event_t min_level, int max_size)
{
	memset(&g_log, 0, sizeof(g_log));
	g_log.min_level = min_level;
	g_log.max_size = (max_size > 0 && max_size <= MAX_LOG_ENTRIES) ? max_size : MAX_LOG_ENTRIES;

	g_log.head = 0;
	g_log.tail = 0;
	g_log.count = 0;
	g_log.shutdown = 0;

	pthread_mutex_init(&g_log.mutex, NULL);
	pthread_cond_init(&g_log.cond, NULL);

	if (log_file) {
		g_log.file_fp = fopen(log_file, "a");
		if (!g_log.file_fp) {
			perror("fopen");
		}
	}

	pthread_create(&g_log.worker, NULL, thread_start, NULL);
}

void log_add_output(FILE *fp)
{
	g_log.extra_fp = fp;
}

void log_write(log_event_t event, const char *module, const char *fmt, ...)
{
	if (!module) {
		module = "unkown module";
	}

	pthread_mutex_lock(&g_log.mutex);

	if (g_log.count >= g_log.max_size) {
		pthread_mutex_unlock(&g_log.mutex);
		return;
	}

	log_item_t *item = &g_log.log_items[g_log.tail];

	clock_gettime(CLOCK_REALTIME, &item->ts);
	item->event = event;
	strncpy(item->module, module, sizeof(item->module) - 1);
	item->module[sizeof(item->module) - 1] = '\0';

	/* 这个va是怎么用的? */
	va_list args;
	va_start(args, fmt);
	vsnprintf(item->msg, sizeof(item->msg), fmt, args);
	va_end(args);

	g_log.tail = (g_log.tail + 1) % MAX_LOG_ENTRIES;
	g_log.count++;

	pthread_cond_signal(&g_log.cond);
	pthread_mutex_unlock(&g_log.mutex);
}

void logger_shutdown(void)
{
	pthread_mutex_lock(&g_log.mutex);
	g_log.shutdown = 1;
	pthread_cond_broadcast(&g_log.cond);
	pthread_mutex_unlock(&g_log.mutex);

	pthread_join(g_log.worker, NULL);

	if (g_log.file_fp)
		fclose(g_log.file_fp);
	pthread_mutex_destroy(&g_log.mutex);
	pthread_cond_destroy(&g_log.cond);
	memset(&g_log, 0, sizeof(g_log));
}
