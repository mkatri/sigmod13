#include <pthread.h>
#include <stdlib.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "cir_queue.h"
extern Trie3 eltire;

//CircularQueue cirq_bfs_queue;
//CircularQueue cirq_used_queue;
//TrieNode3 *free_nodes[30000];
//int *lamda_used[30000];

LinkedList_t* stacks[NUM_THREADS];

void matchEditDIstance(int did, int tid, char *w, int l, int *count,
		TrieNode3 * current, int used, int ind) {

	LinkedList_t * stack = stacks[tid];
	data* dt = (data *) (malloc(sizeof(data)));
	dt->index = ind;
	dt->used = used;
	dt->node = current;
	append(stack, dt);

	while (!isEmpty(stack)) {
		data* temp = (data*) (stack->head.next->data);
		delete_node(stack->head.next);

		if (!(temp->node) || temp->index > l)
			continue;

		while (temp->index < l && (temp->node)) {
			if ((temp->node->next[26]) && temp->used < 3) {
				data* newData = (data *) (malloc(sizeof(data)));
				newData->index = temp->index + 1;
				newData->used = temp->used + 1;
				newData->node = temp->node->next[26];
				append(stack, newData);
			}
			temp->node = temp->node->next[w[temp->index] - BASE_CHAR];
			temp->index = temp->index + 1;
		}
		if (temp->node && temp->index == l) {
			current = temp->node;
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

					if ((queryData->matchedWords[tid]
							& (1 << (segData->wordIndex))) == 0) {
						queryData->matchedWords[tid] |= (1
								<< (segData->wordIndex));

						if (queryData->matchedWords[tid]
								== (1 << (queryData->numWords)) - 1) {
							(*count)++;
						}
					}
					cur = cur->next;
				}
			}
		}
	}

//	while (ind < l && current) {
//		if (current->next[26] && used < 3)
//			matchEditDIstance(did, tid, w, l, count, current->next[26],
//					used + 1, ind + 1);
//		current = current->next[w[ind] - BASE_CHAR];
//		ind++;
//	}

//	if (ind != l)
//		return;

//	if (current && !isEmpty(&(current->list))) {
//		DNode_t *cur = current->list.head.next;
//		SegmentData * segData = (SegmentData *) (cur->data);
//		QueryDescriptor * queryData = segData->parentQuery;
//		while (cur != &(current->list.tail)) {
//			segData = (SegmentData *) (cur->data);
//			queryData = segData->parentQuery;
//
//			if (queryData->docId[tid] != did) {
//				queryData->docId[tid] = did;
//				queryData->matchedWords[tid] = 0;
//			}
//
//			if ((queryData->matchedWords[tid] & (1 << (segData->wordIndex)))
//					== 0) {
//				queryData->matchedWords[tid] |= (1 << (segData->wordIndex));
//
//				if (queryData->matchedWords[tid]
//						== (1 << (queryData->numWords)) - 1) {
//					(*count)++;
//				}
//			}
//			cur = cur->next;
//		}
//	}
}
void matchWord(int did, int tid, char *w, int l, int *count, Trie_t * trie,
		Trie3 * trie3) {
	if (l > 34)
		return;
	matchEditDIstance(did, tid, w, l, count, &trie3->root, 0, 0);
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
