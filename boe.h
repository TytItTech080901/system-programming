#if defined(__linux__) && !defined(_POSIX_C_SOURCE) && !defined(_GNU_SOURCE)
#define _POSIX_C_SOURCE \
	199309L /* best-effort: only takes effect if boe.h is the first include; CLOCK_MONOTONIC */
#endif

#ifndef _BOE_H_
#define _BOE_H_

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* --------------program timing-------------- */
typedef double boe_timer_t;
double boe_get_timeval(void);
#define PROFILE_BEGIN(timer)	(*(timer) = boe_get_timeval())
#define PROFILE_END(timer, ...) printf("%s: %lf ms\n", __VA_ARGS__, boe_get_timeval() - *(timer))

/* --------------dynamic array-------------- */
#ifndef BOE_MALLOC
#define BOE_MALLOC(sz) malloc(sz)
#endif
#ifndef BOE_FREE
#define BOE_FREE(p) free(p)
#endif
#ifndef BOE_REALLOC
#define BOE_REALLOC(p, sz) realloc(p, sz)
#endif

typedef struct {
	size_t count;
	size_t capacity;
} _boe_array_header;

#define BOE_INIT_CAPACITY 64

#define boe_array_add(items, x)                                                               \
	do {                                                                                      \
		if ((items) == NULL) {                                                                \
			_boe_array_header *header =                                                       \
				BOE_MALLOC(BOE_INIT_CAPACITY * sizeof(*(items)) + sizeof(_boe_array_header)); \
			if (header == NULL)                                                               \
				break;                                                                        \
			header->count = 0;                                                                \
			header->capacity = BOE_INIT_CAPACITY;                                             \
			(items) = (void *)(header + 1);                                                   \
		}                                                                                     \
		_boe_array_header *header = (_boe_array_header *)(items) - 1;                         \
		if (header->count >= header->capacity) {                                              \
			header->capacity *= 2;                                                            \
			_boe_array_header *new_header = BOE_REALLOC(                                      \
				header, sizeof(*(items)) * header->capacity + sizeof(_boe_array_header));     \
			if (new_header == NULL)                                                           \
				break;                                                                        \
			header = new_header;                                                              \
			(items) = (void *)(header + 1);                                                   \
		}                                                                                     \
		(items)[header->count++] = (x);                                                       \
	} while (0)

#define boe_array_len(items)  ((items) == NULL ? 0 : ((_boe_array_header *)(items) - 1)->count)
#define boe_array_free(items) BOE_FREE((_boe_array_header *)(items) - 1)

/* --------------log print-------------- */
#ifndef BOE_LOG_LEVEL
#define BOE_LOG_LEVEL 3
#endif /* BOE_LOG_LEVEL */

#define BOE_LOG_LV_ERROR 0
#define BOE_LOG_LV_WARN	 1
#define BOE_LOG_LV_INFO	 2
#define BOE_LOG_LV_DEBUG 3

#if BOE_LOG_LEVEL >= BOE_LOG_LV_ERROR /* ── ERROR ── */
#define LOG_ERROR(fmt, ...) \
	fprintf(stderr, "[ERROR] %s line %d: -->  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif

#if BOE_LOG_LEVEL >= BOE_LOG_LV_WARN /* ── WARN ── */
#define LOG_WARN(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if BOE_LOG_LEVEL >= BOE_LOG_LV_INFO /* ── INFO ── */
#define LOG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if BOE_LOG_LEVEL >= BOE_LOG_LV_DEBUG /* ── DEBUG ── */
#define LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

/* ========================BOE_IMPLEMENTATION======================== */
#ifdef BOE_IMPLEMENTATION

#ifndef BOE_TIMER_SOURCE
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
double boe_get_timeval(void)
{
#if defined(_WIN32)
	static LARGE_INTEGER freq = { 0 };
	if (freq.QuadPart == 0)
		QueryPerformanceFrequency(&freq);
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (double)count.QuadPart / (double)freq.QuadPart * 1e3;

#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
	struct timespec ts = { 0 };
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec * 1e-6;

#else
	return (double)clock() / (double)CLOCKS_PER_SEC * 1e3;
#endif
}
#endif /* BOE_TIMER_SOURCE */

#endif /* BOE_IMPLEMENTATION */

#endif /* _BOE_H */
