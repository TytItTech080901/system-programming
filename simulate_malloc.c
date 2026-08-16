#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#define HEAP_SIZE			100 * 1024
#define HEAP_ALLOCATED_SIZE 1024
#define HEAP_FREED_SIZE		1024

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct _Node {
	struct _Node *prev;
	struct _Node *next;
} Doubly_Node;

typedef struct {
	Doubly_Node node;
	void *start;
	size_t size;
} Heap_Chunk;

char heap[HEAP_SIZE] = { 0 };
size_t heap_size = 0;
Heap_Chunk allocated_chunk[HEAP_ALLOCATED_SIZE] = { 0 };
size_t allocated_size = 0;
Heap_Chunk freed_chunk[HEAP_FREED_SIZE] = { 0 };
size_t freed_size = 0;

void *my_malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}
	// void *start = traverse_heap(heap);
	void *start = heap + heap_size;
	heap_size += size;

	Heap_Chunk *heap_chunk = &allocated_chunk[allocated_size++];
	heap_chunk->node.prev = NULL;
	heap_chunk->node.next = NULL;
	heap_chunk->start = start;
	heap_chunk->size = size;

	if (allocated_size > 1) {
		Heap_Chunk *prev_chunk = &allocated_chunk[allocated_size - 2];
		prev_chunk->node.next = &heap_chunk->node;
		heap_chunk->node.prev = &prev_chunk->node;
	}

	return start;
}

void list_insert(Heap_Chunk *List, void *start, size_t size)
{
}

void *traverse_heap(void *addr)
{
	// TODO: traverse_heap is not implemented
}

void my_free(void *addr)
{
	// TODO: free is not implemented
	Doubly_Node *cur = &allocated_chunk[0].node;
	while (cur) {
		Heap_Chunk *chunk = container_of(cur, Heap_Chunk, node);
		if (chunk->start == addr) {
		}
	}
}

void collect_free_list(void *addr)
{
	// TODO: collect_free is not implemented
}

void heap_dump_chunks(void)
{
	for (size_t i = 0; i < allocated_size; i++) {
		printf("start: %p, size: %zu\n", allocated_chunk[i].start, allocated_chunk[i].size);
	}
}

int main()
{
	for (int i = 0; i < 100; i++) {
		my_malloc(i);
	}

	heap_dump_chunks();

	return 0;
}