/*
 * @file bpe.c
 * @brief description
 * @author tytxxc<Yitaotian080901@outlook.com>
 * @date 2026-06-24 12:11
 *
 * <uthash.h> need download to your pc
 */

#define BOE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uthash.h>
#include "boe.h"

typedef struct {
	unsigned char l, r;
} PAIR_T;

typedef struct {
	PAIR_T key;
	size_t value;
	UT_hash_handle hh;
} KV_TABLE_T;

int cmp_func(const void *a, const void *b)
{
	KV_TABLE_T *pa = *(KV_TABLE_T **)a;
	KV_TABLE_T *pb = *(KV_TABLE_T **)b;
	return pb->value - pa->value;
}

int main()
{
	const char *s =
		"How do we construct reliable, portable, efficient, and secure computer systems? An essential component is the computer's operating system — the software that manages a computer's resources. First, the bad news: operating systems concepts are among the most complex in computer science. A modern, general-purpose operating system can exceed 50 million lines of code, or in other words, more than a thousand times longer than this textbook. New operating systems are being written all the time: if you use an e-book reader, tablet, or smartphone, an operating system is managing your device. Given this inherent complexity, we limit our focus to the essential concepts that every computer scientist should know.";

	KV_TABLE_T *freq = NULL;
	PAIR_T *pPairTable = NULL;
	for (int i = 0; i < 256; i++) {
		boe_array_add(pPairTable, (PAIR_T){ .l = i });
	}

	unsigned char *pTokensIn = NULL;
	unsigned char *pTokensOut = NULL;

	size_t uTokensLen = strlen(s);
	for (size_t i = 0; i < uTokensLen; i++) {
		boe_array_add(pTokensIn, (unsigned char)s[i]);
	}

	// LOG_DEBUG("%c", pTokensIn[uTokensLen - 1]);

	for (int i = 0; i < uTokensLen - 1; i += 1) {
		PAIR_T k = { (unsigned char)pTokensIn[i], (unsigned char)pTokensIn[i + 1] };
		KV_TABLE_T *entry = NULL;

		// struct key use HASH_FIND
		HASH_FIND(hh, freq, &k, sizeof(PAIR_T), entry);
		if (entry == NULL) {
			entry = malloc(sizeof(KV_TABLE_T));
			entry->key = k;
			entry->value = 1;
			HASH_ADD(hh, freq, key, sizeof(PAIR_T), entry);
		} else {
			entry->value++;
		}
	}

	KV_TABLE_T maxPair = { 0 };
	for (KV_TABLE_T *pCurrent = freq; pCurrent != NULL; pCurrent = pCurrent->hh.next) {
		if (pCurrent->value > maxPair.value) {
			maxPair.key = pCurrent->key;
			maxPair.value = pCurrent->value;
		}
	}

	LOG_INFO("(%u,%u) => %lu", maxPair.key.l, maxPair.key.r, maxPair.value);

	boe_array_add(pPairTable, maxPair.key);

	size_t uTokensIdx = 0;
	while (uTokensIdx < uTokensLen - 1) {
		PAIR_T tPair = { .l = pTokensIn[uTokensIdx], .r = pTokensIn[uTokensIdx + 1] };
		if (memcmp(&tPair, &maxPair.key, sizeof(maxPair.key)) == 0) {
			boe_array_add(pTokensOut, boe_array_len(pPairTable));
			uTokensIdx += 2;
		} else {
			boe_array_add(pTokensOut, pTokensIn[uTokensIdx]);
			if (uTokensIdx == uTokensLen - 2) {
				boe_array_add(pTokensOut, pTokensIn[uTokensIdx + 1]);
			}
			uTokensIdx++;
		}
	}

	size_t uTokensOutLen = boe_array_len(pTokensOut);
	LOG_INFO("pTokensOut: ");
	for (size_t i = 0; i < uTokensOutLen; i++) {
		printf("%c", (unsigned char)pTokensOut[i]);
	}
	printf("\n");

	/* 排序 */
	// size_t count = HASH_COUNT(freq);
	// KV_TABLE_T **arr = malloc(count * sizeof(KV_TABLE_T *));
	// KV_TABLE_T *cur;
	// size_t i = 0;
	// for (cur = freq; cur != NULL; cur = cur->hh.next) {
	// 	arr[i++] = cur;
	// }

	// qsort(arr, count, sizeof(KV_TABLE_T *), cmp_func);
	// for (size_t i = 0; i < 10; i++) {
	// 	printf("(%d,%d) => %zu\n", arr[i]->key.l, arr[i]->key.r, arr[i]->value);
	// }

	// free(arr);

	return 0;
}
