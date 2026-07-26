/*
 * @file xordup.c
 * @brief description
 * @author tytxxc<Yitaotian080901@outlook.com>
 * @date 2026-06-16 23:04
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int num[200];

int main()
{
	srand(time(NULL));
	for (int i = 0; i <= 100; i++) {
		num[i] = i;
	}

	int dupnum = rand() % 100 + 1;

	num[101] = dupnum;

	int x = 0;
	for (int i = 1; i < 102; i++) {
		x ^= i;
	}

	for (int i = 0; i < 102; i++) {
		x ^= num[i];
	}

	printf("%d\n", x);
}