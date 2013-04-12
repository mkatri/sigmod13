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
//				&& qTrie->done[tid] != did[0] //TODO not sure about this one
				) {

			qTrie->done[tid] = did[0]; //TODO this is the rest :D
			DNode_t *cur = qTrie->list.head.next;
			SegmentData * segData = (SegmentData *) (cur->data);
			QueryDescriptor * queryData = segData->parentQuery;
			int ok = 0, tmp = 0, d;
			while (cur != &(qTrie->list.tail)) {
				segData = (SegmentData *) (cur->data);
				queryData = segData->parentQuery;
				tmp++;
				for (d = 0; d < task_size; d++) {
					if ((dTrie->fingerPrint & (((uint64_t) 1) << d)) != 0) {
						if (queryData->docId[tid][d] != did[d]) {
							queryData->docId[tid][d] = did[d];
							queryData->matchedWords[tid][d] = 0;
						}

						if ((queryData->matchedWords[tid][d]
								& (1 << (segData->wordIndex))) == 0) {
							ok = 1;
							queryData->matchedWords[tid][d] |= (1
									<< (segData->wordIndex));

							if (queryData->matchedWords[tid][d]
									== (1 << (queryData->numWords)) - 1) {
								count[d]++;
								append_with_pool(&results[d],
										(void *) (uintptr_t) queryData->queryId,
										pool);
							}
						}
					}
				}
				cur = cur->next;
			}
			if (!ok)
				overhead[tid] += tmp;
			total[tid] += tmp;
		}

		int i;
		for (i = 0; i < 26; i++) {
			if (dTrie->next[i] && dTrie->next[i]->docId == did[0]) {
				if (qTrie->next[26]) {
					dtrieQueue[tid][p] = dTrie->next[i];
					qtrieQueue[tid][p++] = qTrie->next[26];
					size++;
				}
				if (qTrie->next[i]) {
					dtrieQueue[tid][p] = dTrie->next[i];
					qtrieQueue[tid][p++] = qTrie->next[i];
					size++;
				}
			}
		}
	}
}
