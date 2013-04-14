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
TrieNode_t2 * dtrieQueue[INIT_QUEUE_SIZE ];
TrieNode3 * qtrieQueue[INIT_QUEUE_SIZE ];

extern long long overhead;
extern long long total;

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
//matchTrie(doc_desc_array[0]->docId, matchCount, task_size, &(dtrie->root),
//			&(eltire->root), &qresult);
void matchTrie(int did, int *count, int task_size, TrieNode_t2 *dTrie,
		TrieNode3 *qTrie, LinkedList_t *results) {
	dtrieQueue[0] = dTrie;
	qtrieQueue[0] = qTrie;
	int size = 1;
	int ptr = 0;
	int p = 1;
	while (size != 0) {
		qTrie = qtrieQueue[ptr];
		dTrie = dtrieQueue[ptr];
		ptr++;
		size--;

//		if (dTrie->docId != did)
//			return;

		if (dTrie->terminal == 1 && qTrie && !isEmpty(&(qTrie->list))
				&& (qTrie->done != did
						|| (qTrie->done_bitmask
								!= (((uint64_t) 1L << task_size) - 1)))) {

			if (qTrie->done != did) {
				qTrie->done_bitmask = 0;
				qTrie->done = did;
			}

			long notDoneFingerPrint = dTrie->fingerPrint & ~qTrie->done_bitmask;

			if (notDoneFingerPrint != 0) {
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData = (SegmentData *) (cur->data);
				QueryDescriptor * queryData = segData->parentQuery;
				int ok = 0, tmp = 0;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;
					tmp++;
					if (queryData->thSpec.docId != did) {
						queryData->thSpec.docId = did;
						memset(queryData->thSpec.docsMatchedWord, 0,
								5 * sizeof(long));
					}

					long localNotDoneFingerPrint =
							notDoneFingerPrint
									& ~queryData->thSpec.docsMatchedWord[segData->wordIndex];

					if (localNotDoneFingerPrint != 0) {
						ok = 1;
						int w;
						long oldQueryStatus, newQueryStatus;
						oldQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							oldQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						//TODO check this bitch? :D
						queryData->thSpec.docsMatchedWord[segData->wordIndex] |=
								notDoneFingerPrint;

						newQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							newQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						if (oldQueryStatus == 0 && newQueryStatus != 0) {
							append(results,
									(void *) (uintptr_t) queryData);
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
				if (!ok)
					overhead += tmp;
				total += tmp;

			}

			qTrie->done_bitmask |= dTrie->fingerPrint;
		}

		int band = (qTrie->qmask & dTrie->dmask), dmask = dTrie->dmask, ind;

		TrieNode3* star = qTrie->next[26];

		while ((ind = bsf(dmask)) > -1) {
			dmask ^= (1 << ind);
			if (star) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = star;
				size++;
			}
			if (band & (1 << ind)) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = qTrie->next[ind];
				size++;
			}
		}

	}
}
