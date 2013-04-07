#include <pthread.h>
#include <stdlib.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "dyn_array.h"
extern int cnttt;
extern Trie3 eltire;

int hammingDistance(char *a, char *b, int n, int max) {
	int mismatch = 0;
	int i;

	for (i = 0; i < n; i++) {
		if ((a[i] - b[i]) != 0)
			mismatch++;

		/* TODO test performance */
		if (mismatch > max)
			return mismatch;
	}

	return mismatch;
}

inline int preCheck(int na, int nb, int dist) {
	if (na == 0)
		return nb;
	if (nb == 0)
		return na;
	if (abs(na - nb) > dist)
		return dist + 1;
	return 0;
}

inline char minimum(char a, char b) {
	return (a < b) ? a : b;
}
int editDistance(int tid, char* a, int na, char* b, int nb, int dist) {
	static char dp[NUM_THREADS][35][33];
	unsigned char ia, ib, strt, min;
	if (abs(na - nb) > dist)
		return dist + 1;
	for (ib = 0; ib <= nb; ib++)
		dp[tid][0][ib] = ib;
	for (ia = 1; ia <= na; ia++) {
		dp[tid][ia][0] = ia;
		min = dist + 1;
		strt = ia - dist > 1 ? ia - dist : 1;
		if (strt > 1)
			dp[tid][ia][strt - 1] = dist + 1;
		for (ib = strt; ib <= ia + dist && ib <= nb; ib++) {
			if (a[ia - 1] == b[ib - 1])
				dp[tid][ia][ib] = dp[tid][ia - 1][ib - 1];
			else {
				dp[tid][ia][ib] = 1
						+ minimum(
								minimum(dp[tid][ia - 1][ib],
										dp[tid][ia][ib - 1]),
								dp[tid][ia - 1][ib - 1]);
			}
			min = minimum(min, dp[tid][ia][ib]);
		}
		if (min > dist)
			return min;
		if (ia + dist + 1 <= nb)
			dp[tid][ia][ia + dist + 1] = dist + 1;
	}
	return dp[tid][na][nb];
}
inline int min(int a, int b) {
	if (a <= b)
		return a;
	return b;
}
inline int max(int a, int b) {
	if (a >= b)
		return a;
	return b;
}
#ifdef PROFILER
void handleQuery(int tid, int did, DNode_t *cur, int z, int type, int i, int j,
		char *w, int l, int *count)
#else
inline void __attribute__((always_inline)) handleQuery(int tid, int did,
		DNode_t *cur, int z, int type, int i, int j, char *w, int l, int *count)
#endif
{
	if (type == MT_EDIT_DIST) {
		//		cnttt++;
//		//2nd_lvl_trie_node
//		TrieNode_t* trie_node = (TrieNode_t*) cur->data;
//		LinkedList_t* _2nd_lvl_list = trie_node->edit_dist_list[z];
//
//		if (!isEmpty(_2nd_lvl_list)) {
//
//			DNode_t* cur2 = _2nd_lvl_list->head.next;
//
//			SegmentData * segData = (SegmentData *) (cur2->data);
//			QueryDescriptor * queryData = segData->parentQuery;
//			int d1;
//			if (!(d1 = preCheck(i,
//					segData->startIndex - queryData->words[segData->wordIndex],
//					trie_node->max_dist[z]))) {
//				d1 += editDistance(tid, w, i,
//						queryData->words[segData->wordIndex],
//						segData->startIndex
//								- queryData->words[segData->wordIndex],
//						trie_node->max_dist[z] - d1);
//			}
//			if (d1 <= trie_node->max_dist[z]) {
//				int tmp = 0;
//				if (!(tmp = preCheck(l - j,
//						queryData->words[segData->wordIndex + 1]
//								- segData->startIndex - (j - i),
//						trie_node->max_dist[z] - d1))) {
//					d1 += editDistance(tid, w + j, l - j,
//							segData->startIndex + j - i,
//							queryData->words[segData->wordIndex + 1]
//									- segData->startIndex - (j - i),
//							trie_node->max_dist[z] - d1);
//				} else
//					d1 += tmp;
//			}
//
//			if(d1>trie_node->max_dist[z])return;
//
//			while (cur2 != &(_2nd_lvl_list->tail)) {
//
//				segData = (SegmentData *) (cur2->data);
//				queryData = segData->parentQuery;
//
//				if (queryData->docId[tid] != did) {
//					queryData->docId[tid] = did;
//					queryData->matchedWords[tid] = 0;
//				}
//
//				if ((queryData->matchedWords[tid] & (1 << (segData->wordIndex)))
//						== 0 && d1 <= queryData->matchDistance) {
//					queryData->matchedWords[tid] |= (1 << (segData->wordIndex));
//
//					if (queryData->matchedWords[tid]
//							== (1 << (queryData->numWords)) - 1) {
//						(*count)++;
//					}
//				}
//
//				cur2 = cur2->next;
//			}
//		}

	} else {
		SegmentData * segData = (SegmentData *) (cur->data);
		QueryDescriptor * queryData = segData->parentQuery;

		if (queryData->docId[tid] != did) {
			queryData->docId[tid] = did;
			queryData->matchedWords[tid] = 0;
		}

		if (((queryData->matchedWords[tid]) & (1 << (segData->wordIndex)))) {
			cur = cur->next;

			return;
		}

		if (type == MT_HAMMING_DIST) {
			if (i == segData->startIndex - queryData->words[segData->wordIndex]
					&& (l - j)
							== queryData->words[segData->wordIndex + 1]
									- segData->startIndex - (j - i)) {
				int d1 = hammingDistance(w,
						queryData->words[segData->wordIndex], i,
						queryData->matchDistance);
				if (d1 <= queryData->matchDistance) {
					d1 += hammingDistance(w + j,
							queryData->words[segData->wordIndex] + j, l - j,
							queryData->matchDistance - d1);

					if (d1 <= queryData->matchDistance) {
						queryData->matchedWords[tid] |= (1
								<< (segData->wordIndex));

						if (queryData->matchedWords[tid]
								== (1 << (queryData->numWords)) - 1) {
							(*count)++;
						}
					}
				}
			}
		} else if (i == 0 && j == l) { // Exact matching must be done from the start of the word only
			queryData->matchedWords[tid] |= (1 << (segData->wordIndex));

			if (queryData->matchedWords[tid]
					== (1 << (queryData->numWords)) - 1) {
				(*count)++;
			}
		}
	}
}
extern int cntz;
 void matchEditDIstance(int did, int tid, char *w, int l, int *count,
		TrieNode3 * current, int used, int ind) {
	while (ind < l && current) {
		if (current->next[26] && used < 3)
			matchEditDIstance(did, tid, w, l, count, current->next[26],
					used + 1, ind + 1);
		current = current->next[w[ind] - BASE_CHAR];
		ind++;
	}

	if (ind != l)
		return;

	if (current && !isEmpty(current->list)) {
		DNode_t *cur = current->list->head.next;
		SegmentData * segData = (SegmentData *) (cur->data);
		QueryDescriptor * queryData = segData->parentQuery;
		while (cur != &(current->list->tail)) {
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
void matchWord(int did, int tid, char *w, int l, int *count, Trie_t * trie,
		Trie3 * trie3) {
	if (l > 34)
		return;
	matchEditDIstance(did, tid, w, l, count, &trie3->root, 0, 0);
	int i = 0;
	for (i = 0; i < l; i++) {
		int j = i;
		TrieNode_t *n = &trie->root;
		while ((n = next_node(n, w[j])) && j < l) {
			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
					&& (i > 0 || n->count[MT_EXACT_MATCH] == 0))
				break;
			j++;
			LinkedList_t * list;
			list = n->list2[l];
			if (!isEmpty(list)) {
				DNode_t *cur = list->head.next;
				while (/*cur->data &&*/cur != &(list->tail)) {
					handleQuery(tid, did, cur, l,
							((SegmentData*) cur->data)->parentQuery->matchType,
							i, j, w, l, count);
					cur = cur->next;
				}
			}
		}
	}
}
