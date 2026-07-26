#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_SIZE 32

int main()
{
	int *ptr = NULL;
	ptr = malloc(sizeof(int) * LIST_SIZE);
    if (ptr == NULL) {
		fprintf(stderr, "malloc failed");
		exit(EXIT_FAILURE);
	}

	memset(ptr, 0x0, sizeof(int) * LIST_SIZE);

	for (int i = 0; i < LIST_SIZE; i++) {
		ptr[i] = rand() % 256;
	}

	for (int i = 0; i < LIST_SIZE; i++) {
		printf("ptr[%d] = %d\n", i, ptr[i]);
	}

	free(ptr);

	return 0;
}
