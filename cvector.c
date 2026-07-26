#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int *item;
	size_t count;
	size_t capacity;
} Number;

#define di_append(xs, x)                                                \
	do {                                                                \
		if (xs.count >= xs.capacity) {                                  \
			if (xs.capacity == 0)                                       \
				xs.capacity = 256;                                      \
			else                                                        \
				xs.capacity *= 2;                                       \
			xs.item = realloc(xs.item, xs.capacity * sizeof(*xs.item)); \
		}                                                               \
		xs.item[xs.count++] = x;                                        \
	} while (0)

int main()
{
	Number xs = { 0 };
	for (int x = 0; x < 10; x++) {
		di_append(xs, x);
	}

	for (size_t i = 0; i < xs.count; i++) {
		printf("%d\n", xs.item[i]);
	}

	return 0;
}