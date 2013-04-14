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

//extern long long overhead[NUM_THREADS];
//extern long long total[NUM_THREADS];

inline int bsf(int bitmask) {
	int first = 0;
	int isZero = -1;
	asm(
			"bsf %1, %0\n\t"
			"cmove %2, %0"
			:"=r"(first)
			:"g"(bitmask), "rm"(isZero)
			:);

	return first;
}

void matchTrie(int *did, int tid, int *count, int task_size, TrieNode_t2 *dTrie,
		TrieNode3 *qTrie, LinkedList_t *results, LinkedList_t *pool) {
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
				&& (qTrie->done[tid] != did[0]
						|| (qTrie->done_bitmask[tid]
								!= (((uint64_t) 1L << task_size) - 1)))) {

			if (qTrie->done[tid] != did[0]) {
				qTrie->done_bitmask[tid] = 0;
				qTrie->done[tid] = did[0];
			}

			int notDoneFingerPrint = dTrie->fingerPrint
					& ~qTrie->done_bitmask[tid];
			int d;
			while ((d = bsf(notDoneFingerPrint)) > -1) {
				notDoneFingerPrint ^= (1 << d);
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData = (SegmentData *) (cur->data);
				QueryDescriptor * queryData = segData->parentQuery;
				//					int ok = 0, tmp = 0;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;
					//						tmp++;
					if (queryData->thSpec[tid].docId[d] != did[d]) {
						queryData->thSpec[tid].docId[d] = did[d];
						queryData->thSpec[tid].matchedWords[d] = 0;
					}

					if ((queryData->thSpec[tid].matchedWords[d]
							& (1 << (segData->wordIndex))) == 0) {
						//							ok = 1;
						queryData->thSpec[tid].matchedWords[d] |= (1
								<< (segData->wordIndex));

						if (queryData->thSpec[tid].matchedWords[d]
								== (1 << (queryData->numWords)) - 1) {
							count[d]++;
							append_with_pool(&results[d],
									(void *) (uintptr_t) queryData->queryId,
									pool);
						}
					}
					cur = cur->next;
				}
				//					if (!ok)
				//						overhead[tid] += tmp;
				//					total[tid] += tmp;
			}

			qTrie->done_bitmask[tid] |= dTrie->fingerPrint;
		}

		int band = (qTrie->qmask & dTrie->dmask), dmask = dTrie->dmask, ind;

		TrieNode3* star = qTrie->next[26];

		while ((ind = bsf(dmask)) > -1) {
			dmask ^= (1 << ind);
			if (star) {
				dtrieQueue[tid][p] = dTrie->next[ind];
				qtrieQueue[tid][p++] = star;
				size++;
			}
			if (band & (1 << ind)) {
				dtrieQueue[tid][p] = dTrie->next[ind];
				qtrieQueue[tid][p++] = qTrie->next[ind];
				size++;
			}
		}

	}
}
