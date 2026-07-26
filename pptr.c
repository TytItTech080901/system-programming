/*
 * @file pptr.c
 * @brief temp file
 * @author tytxxc<Yitaotian080901@outlook.com>
 * @date 2026-06-17 19:05
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	size_t count;
	size_t capacity;
} Header;

#define INIT_CAPACITY 256

int main()
{
	Header *header = malloc(sizeof(int) * INIT_CAPACITY + sizeof(Header));
	header->capacity = INIT_CAPACITY;
	header->count = 0;
	int *items = (int *)(header + 1);
	for (int i = 0; i < 10; i++) {
		items[header->count++] = i;
	}

	for (size_t i = 0; i < header->count; i++) {
		printf("%d\n", items[i]);
	}

	return 0;
}
