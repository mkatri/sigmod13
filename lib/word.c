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

inline long bsfl(long bitmask) {
	long first = 0;
	long isZero = -1;
	asm(
			"bsf %1, %0\n\t"
			"cmove %2, %0"
			:"=r"(first)
			:"g"(bitmask), "rm"(isZero)
			:);

	return first;
}

void matchTrie(int did, int tid, int *count, int task_size, TrieNode_t2 *dTrie,
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
				&& (qTrie->done[tid] != did
						|| (qTrie->done_bitmask[tid]
								!= (((uint64_t) 1L << task_size) - 1)))) {

			if (qTrie->done[tid] != did) {
				qTrie->done_bitmask[tid] = 0;
				qTrie->done[tid] = did;
			}

			long notDoneFingerPrint = dTrie->fingerPrint
					& ~qTrie->done_bitmask[tid];

			if (notDoneFingerPrint != 0) {
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData = (SegmentData *) (cur->data);
				QueryDescriptor * queryData = segData->parentQuery;
				//					int ok = 0, tmp = 0;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;
					//						tmp++;
					if (queryData->thSpec[tid].docId != did) {
						queryData->thSpec[tid].docId = did;
						memset(queryData->thSpec[tid].docsMatchedWord, 0,
								5 * sizeof(long));
					}

					long localNotDoneFingerPrint =
							notDoneFingerPrint
									& ~queryData->thSpec[tid].docsMatchedWord[segData->wordIndex];

					if (localNotDoneFingerPrint != 0) {

						int w;
						long oldQueryStatus, newQueryStatus;
						oldQueryStatus =
								queryData->thSpec[tid].docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							oldQueryStatus &=
									queryData->thSpec[tid].docsMatchedWord[w];

						//TODO check this bitch? :D
						queryData->thSpec[tid].docsMatchedWord[segData->wordIndex] |=
								notDoneFingerPrint;

						newQueryStatus =
								queryData->thSpec[tid].docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							newQueryStatus &=
									queryData->thSpec[tid].docsMatchedWord[w];

						if (oldQueryStatus == 0 && newQueryStatus != 0) {
							append_with_pool(results,
									(void *) (uintptr_t) queryData, pool);
						}

						long d;
						while ((d = bsfl(localNotDoneFingerPrint)) > -1) {
							localNotDoneFingerPrint ^= (1L << d);
							if (((oldQueryStatus & (1L << d)) == 0)
									&& ((newQueryStatus & (1L << d)) != 0))
								count[d]++;
						}
					}
					cur = cur->next;
				}
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
