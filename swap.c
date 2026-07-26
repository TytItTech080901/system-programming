#include <stdio.h>

int main()
{
	int a = 12;
	int b = 20;
	printf("%d %d\n", a, b);
	a ^= b; // a = 12^20, b = 20
	b ^= a; // a = 12^20, b = 20^12^20
	a ^= b;
	printf("%d %d\n", a, b);

	return 0;
}