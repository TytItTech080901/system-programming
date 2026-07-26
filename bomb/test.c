#include <stdio.h>

long test(unsigned long len, long a)
{
	long result = 0;
	long i;
	if (len <= 10) {
		for (i = 0; i < len; i++) {
			result += a;
			a++;
		}
	}

	return result;
}

int main()
{
	return 0;
}