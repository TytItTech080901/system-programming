#ifndef __SHADOWHEADER_H
#define __SHADOWHEADER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	size_t count;
	size_t capacity;
} __Header;

#define INIT_CAPACITY 2

#define array_push(items, x)                                                                \
	do {                                                                                    \
		if ((items) == NULL) {                                                              \
			__Header *header = malloc(INIT_CAPACITY * sizeof(*(items)) + sizeof(__Header)); \
			if (header == NULL)                                                             \
				break;                                                                      \
			header->count = 0;                                                              \
			header->capacity = INIT_CAPACITY;                                               \
			(items) = (void *)(header + 1);                                                 \
		}                                                                                   \
		__Header *header = (__Header *)(items) - 1;                                         \
		if (header->count >= header->capacity) {                                            \
			header->capacity *= 2;                                                          \
			__Header *new_header =                                                          \
				realloc(header, sizeof(*(items)) * header->capacity + sizeof(__Header));    \
			if (new_header == NULL)                                                         \
				break;                                                                      \
			header = new_header;                                                            \
			(items) = (void *)(header + 1);                                                 \
		}                                                                                   \
		(items)[header->count++] = (x);                                                     \
	} while (0)

#define array_len(items) ((items) == NULL ? 0 : ((__Header *)(items) - 1)->count)
#define array_free(items) free((__Header *)(items) - 1)

#endif