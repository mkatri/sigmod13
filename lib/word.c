#include <pthread.h>
#include <stdlib.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "word.h"

extern long long global_time;

void matchEditDIstance(int did, int tid, char *w, int l, int *count,
		TrieNode3 * current, int used, int ind, LinkedList_t * list,
		long long word_time) {
	while (ind < l && current) {
		if (current->next[26] && used < 3)
			matchEditDIstance(did, tid, w, l, count, current->next[26],
					used + 1, ind + 1, list, word_time);
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
				}
			}
			cur = cur->next;
		}
	}
}
void matchWord(int did, int tid, char *w, int l, int *count, Trie3 * trie3,
		LinkedList_t* list, long long word_time) {
	matchEditDIstance(did, tid, w, l, count, &trie3->root, 0, 0, list,
			word_time);
}
