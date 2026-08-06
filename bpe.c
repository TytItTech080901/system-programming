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
	int l, r;
} Pair_t;

typedef struct {
	Pair_t key;
	size_t value;
	UT_hash_handle hh;
} KV_table_t;

int cmp_func(const void *a, const void *b)
{
	KV_table_t *pa = *(KV_table_t **)a;
	KV_table_t *pb = *(KV_table_t **)b;
	return pb->value - pa->value;
}

int main()
{
	const char *s =
		"How do we construct reliable, portable, efficient, and secure computer systems? An essential component is the computer's operating system — the software that manages a computer's resources. First, the bad news: operating systems concepts are among the most complex in computer science. A modern, general-purpose operating system can exceed 50 million lines of code, or in other words, more than a thousand times longer than this textbook. New operating systems are being written all the time: if you use an e-book reader, tablet, or smartphone, an operating system is managing your device. Given this inherent complexity, we limit our focus to the essential concepts that every computer scientist should know.";

	KV_table_t *freq = NULL;
	Pair_t *pair_table = NULL;
	for (int i = 0; i < 256; i++) {
		boe_array_add(pair_table, (Pair_t){ .l = i });
	}

	unsigned int *tokens_in = NULL;
	unsigned int *tokens_out = NULL;

	size_t tokens_len = strlen(s);
	for (size_t i = 0; i < tokens_len; i++) {
		boe_array_add(tokens_in, (unsigned int)s[i]);
	}

	// LOG_DEBUG("%c", tokens_in[tokens_len - 1]);

	int iter_cnt = 0;
	for (;;) {
		if (tokens_len <= 1)
			break;

		for (int i = 0; i < tokens_len - 1; i += 1) {
			Pair_t k = { tokens_in[i], tokens_in[i + 1] };
			KV_table_t *entry = NULL;

			// struct key use HASH_FIND
			HASH_FIND(hh, freq, &k, sizeof(Pair_t), entry);
			if (entry == NULL) {
				entry = malloc(sizeof(KV_table_t));
				entry->key = k;
				entry->value = 1;
				HASH_ADD(hh, freq, key, sizeof(Pair_t), entry);
			} else {
				entry->value++;
			}
		}

		KV_table_t max_pair = { 0 };
		for (KV_table_t *cur = freq; cur != NULL; cur = cur->hh.next) {
			if (cur->value > max_pair.value) {
				max_pair.key = cur->key;
				max_pair.value = cur->value;
			}
		}

		// LOG_INFO("(%u,%u) => %lu", max_pair.key.l, max_pair.key.r, max_pair.value);

		boe_array_add(pair_table, max_pair.key);

		size_t tokens_idx = 0;
		while (tokens_idx < tokens_len) {
			if (tokens_idx + 1 < tokens_len) {
				Pair_t tPair = { .l = tokens_in[tokens_idx], .r = tokens_in[tokens_idx + 1] };
				if (memcmp(&tPair, &max_pair.key, sizeof(Pair_t)) == 0) {
					boe_array_add(tokens_out, boe_array_len(pair_table) - 1);
					tokens_idx += 2;
					continue;
				}
				boe_array_add(tokens_out, tokens_in[tokens_idx]);
				tokens_idx += 1;
			}

			// size_t tokens_out_len = boe_array_len(tokens_out);
			// LOG_INFO("tokens_out: ");
			// for (size_t i = 0; i < tokens_out_len; i++) {
			// 	if (tokens_out[i] < 256)
			// 		printf("%c", tokens_out[i]);
			// 	else
			// 		printf("<%u>", tokens_out[i]);
			// }
			// printf("\n");

			KV_table_t *cur, *tmp;
			HASH_ITER(hh, freq, cur, tmp)
			{
				HASH_DEL(freq, cur);
				free(cur);
			}

			unsigned int *t = tokens_in;
			tokens_in = tokens_out;
			tokens_out = t;

			boe_array_free(tokens_out);
			tokens_out = NULL;
			tokens_len = boe_array_len(tokens_in);
			LOG_INFO("iter[%d] tokens_len: %zu", iter_cnt, tokens_len);
			iter_cnt++;
		}
	}

	return 0;
}
