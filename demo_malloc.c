#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main()
{
	const char *message = "this is heap message";
	const size_t msg_size = strlen(message);
	char *memory = (char *)sbrk(msg_size);
	strcpy(memory, message);
	const char *second_message = "this is second message";
	const size_t second_msg_size = strlen(second_message);
	char *second_memory = (char *)sbrk(second_msg_size);
	strcpy(second_memory, second_message);
	brk(memory);

	const char *over_message = "this is over message";
	const size_t over_msg_size = strlen(over_message);
	char *over_memory = (char *)sbrk(over_msg_size);
	strcpy(over_memory, over_message);

	return 0;
}