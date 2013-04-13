#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "word.h"
char matched[QDESC_MAP_SIZE ][6];
TrieNode_t2 * dtrieQueue[NUM_THREADS][INIT_QUEUE_SIZE ];
TrieNode3 * qtrieQueue[NUM_THREADS][INIT_QUEUE_SIZE ];

extern long long overhead[NUM_THREADS];
extern long long total[NUM_THREADS];

inline int bsf(int bitmask) {
	char isZero = 0;
	int first = 0;

	//TODO read from memroy directly?
	asm volatile(
			"bsf %2, %1\n\t"
			"sete %0"
			:"=r"(isZero), "=r"(first)
			:"r"(bitmask)
			:);

	if (isZero)
		return -1;
	return first;
}

void matchTrie(int did, int tid, int *count, TrieNode_t2 * dTrie,
		TrieNode3 * qTrie, LinkedList_t *results, LinkedList_t *pool) {
	dtrieQueue[tid][0] = dTrie;
	qtrieQueue[tid][0] = qTrie;
	int size = 1;
	int ptr = 0;
	int p = 1;
	while (size != 0) {
		qTrie = qtrieQueue[tid][ptr];
		dTrie = dtrieQueue[tid][ptr];
		ptr++;
		size--;

//		if (dTrie->docId != did)
//			return;

		if (dTrie->terminal == 1 && qTrie && !isEmpty(&(qTrie->list))
				&& qTrie->done[tid] != did) {

			qTrie->done[tid] = did;
			DNode_t *cur = qTrie->list.head.next;
			SegmentData * segData = (SegmentData *) (cur->data);
			QueryDescriptor * queryData = segData->parentQuery;
			int ok = 0, tmp = 0;
			while (cur != &(qTrie->list.tail)) {
				segData = (SegmentData *) (cur->data);
				queryData = segData->parentQuery;
				tmp++;
				if (queryData->docId[tid] != did) {
					queryData->docId[tid] = did;
					queryData->matchedWords[tid] = 0;
				}

				if ((queryData->matchedWords[tid] & (1 << (segData->wordIndex)))
						== 0) {
					ok = 1;
					queryData->matchedWords[tid] |= (1 << (segData->wordIndex));

					if (queryData->matchedWords[tid]
							== (1 << (queryData->numWords)) - 1) {
						(*count)++;
						append_with_pool(results,
								(void *) (uintptr_t) queryData->queryId, pool);
					}
				}
				cur = cur->next;
			}

			if (!ok)
				overhead[tid] += tmp;
			total[tid] += tmp;
		}

		int band = (qTrie->qmask & dTrie->dmask), dmask = dTrie->dmask, index;

		TrieNode3* star = qTrie->next[26];

		if (star) {
			while ((index = bsf(dmask)) > -1) {
				dmask ^= (1 << index);
				dtrieQueue[tid][p] = dTrie->next[index];
				qtrieQueue[tid][p++] = star;
				size++;
			}
		}

		while ((index = bsf(band)) > -1) {
			band ^= (1 << index);
			dtrieQueue[tid][p] = dTrie->next[index];
			qtrieQueue[tid][p++] = qTrie->next[index];
			size++;
		}

	}
}
