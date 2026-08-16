#include <stdio.h>
#include <string.h>

typedef struct _Node {
	struct _Node *prev;
	struct _Node *next;
} Node;

struct free_list {
	struct Node *prev;
	struct Node *next;
	int used_size;
	int pay_size;
	int data[];
};

void *my_malloc(int size)
{
}


int main()
{
	int mempool[640];
	memset(mempool, 0x0, sizeof(mempool));
	struct free_list *demo;


	// memset(demo, 0x0, sizeof(demo));
	printf("size of free_list is %ld\n", sizeof(*demo));
}