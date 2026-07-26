/*
 * @file shadowheader.c
 * @brief Intrusive header implementation for dynamic array
 * @author tytxxc<Yitaotian080901@outlook.com>
 * @date 2026-6-16 22:04
 * 
 * see more detail on:
 * https://github.com/troydhanson/uthash/blob/master/src/utarray.h
 * https://www.bilibili.com/video/BV18qfaBWEH2/
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	size_t count;
	size_t capacity;
} Header;

#define INIT_CAPACITY 2

#define array_push(items, x)                                                            \
	do {                                                                                \
		if ((items) == NULL) {                                                          \
			Header *header = malloc(INIT_CAPACITY * sizeof(*(items)) + sizeof(Header)); \
			if (header == NULL)                                                         \
				break;                                                                  \
			header->count = 0;                                                          \
			header->capacity = INIT_CAPACITY;                                           \
			(items) = (void *)(header + 1);                                             \
		}                                                                               \
		Header *header = (Header *)(items) - 1;                                         \
		if (header->count >= header->capacity) {                                        \
			header->capacity *= 2;                                                      \
			Header *new_header =                                                        \
				realloc(header, sizeof(*(items)) * header->capacity + sizeof(Header));  \
			if (new_header == NULL)                                                     \
				break;                                                                  \
			header = new_header;                                                        \
			(items) = (void *)(header + 1);                                             \
		}                                                                               \
		(items)[header->count++] = (x);                                                 \
	} while (0)

#define array_len(items) ((items) == NULL ? 0 : ((Header *)(items) - 1)->count)
#define array_free(items) free((Header *)(items) - 1)

int main()
{
	int *items = NULL;
	array_push(items, 222.0);
	array_push(items, 22.0);
	array_push(items, 2.0);
	array_push(items, 69.0);

	size_t len = array_len(items);
	for (size_t i = 0; i < len; i++) {
		printf("%d\n", items[i]);
	}

	array_free(items);
	return 0;
}
