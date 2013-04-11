#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "word.h"

void matchEditDIstance(int did, int tid, char *w, int l, int *count,
		TrieNode3 * current, int ind, LinkedList_t *results, LinkedList_t *pool) {
	while (ind < l && current) {
		if (current->next[26])
			matchEditDIstance(did, tid, w, l, count, current->next[26], ind + 1,
					results, pool);
		current = current->next[w[ind] - BASE_CHAR];
		ind++;
	}

	if (ind != l)
		return;

	if (current && !isEmpty(&(current->list))) {
		DNode_t *cur = current->list.head.next;
		SegmentData * segData = (SegmentData *) (cur->data);
		QueryDescriptor * queryData = segData->parentQuery;
		while (cur != &(current->list.tail)) {
			segData = (SegmentData *) (cur->data);
			queryData = segData->parentQuery;

			if (queryData->docId[tid] != did) {
				queryData->docId[tid] = did;
				queryData->matchedWords[tid] = 0;
			}

			if ((queryData->matchedWords[tid] & (1 << (segData->wordIndex)))
					== 0) {
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
	}
}
TrieNode_t2 * dtrieQueue[NUM_THREADS][INIT_QUEUE_SIZE];
TrieNode3 * qtrieQueue[NUM_THREADS][INIT_QUEUE_SIZE];
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
//		printf("%d %d %d\n",ptr,p,size);
		if (dTrie->docId != did)
			return;

		if (dTrie->terminal == 1 && qTrie && !isEmpty(&(qTrie->list))) {
			if (qTrie->done[tid] != did) {
				qTrie->done[tid] = did;
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData = (SegmentData *) (cur->data);
				QueryDescriptor * queryData = segData->parentQuery;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;

					if (queryData->docId[tid] != did) {
						queryData->docId[tid] = did;
						queryData->matchedWords[tid] = 0;
					}

					if ((queryData->matchedWords[tid]
							& (1 << (segData->wordIndex))) == 0) {
						queryData->matchedWords[tid] |= (1
								<< (segData->wordIndex));

						if (queryData->matchedWords[tid]
								== (1 << (queryData->numWords)) - 1) {
							(*count)++;
							append_with_pool(results,
									(void *) (uintptr_t) queryData->queryId,
									pool);
						}
					}
					cur = cur->next;
				}
			}
		}

		int i;
		for (i = 0; i < 26; i++) {
			if (dTrie->next[i] != 0 && dTrie->next[i]->docId == did) {
				if (qTrie->next[26]) {
					dtrieQueue[tid][p] = dTrie->next[i];
					qtrieQueue[tid][p++] = qTrie->next[26];
					size++;
//					p%=INIT_QUEUE_SIZE;
//					matchTrie(did, tid, count, dTrie->next[i], qTrie->next[26],
//							results, pool);
				}
				if (qTrie->next[i]) {
					dtrieQueue[tid][p] = dTrie->next[i];
					qtrieQueue[tid][p++] = qTrie->next[i];
					size++;
//					p%=INIT_QUEUE_SIZE;
//					matchTrie(did, tid, count, dTrie->next[i], qTrie->next[i],
//							results, pool);
				}
			}
		}
	}
}

void matchWord(int did, int tid, char *w, int l, int *count, Trie_t * trie,
		Trie3 * trie3, LinkedList_t *results, LinkedList_t *pool) {
	if (l > 34)
		return;
	matchEditDIstance(did, tid, w, l, count, &trie3->root, 0, results, pool);
//	int i = 0;
//	for (i = 0; i < l; i++) {
//		int j = i;
//		TrieNode_t *n = &trie->root;
//		while ((n = next_node(n, w[j])) && j < l) {
//			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
//					&& (i > 0 || n->count[MT_EXACT_MATCH] == 0))
//				break;
//			j++;
//			LinkedList_t * list;
//			list = n->list2[l];
//			if (!isEmpty(list)) {
//				DNode_t *cur = list->head.next;
//				while (/*cur->data &&*/cur != &(list->tail)) {
//					handleQuery(tid, did, cur, l,
//							((SegmentData*) cur->data)->parentQuery->matchType,
//							i, j, w, l, count);
//					cur = cur->next;
//				}
//			}
//		}
//	}
}
