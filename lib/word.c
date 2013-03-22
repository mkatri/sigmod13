#include <pthread.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "dyn_array.h"

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

int editDistance(int tid, char* a, int na, char* b, int nb, int dist) {
	int oo = 0x7FFFFFFF;

	static int T[NUM_THREADS][2][100];

	int ia, ib;
//	for (ia = 0; ia < na; ia++)
//		putchar(a[ia]);
//	putchar('\n');
//	for (ib = 0; ib < nb; ib++)
//		putchar(b[ib]);
//	putchar('\n');
	int cur = 0, min;
	ia = 0;
	min = 1 << 30;

	for (ib = 0; ib <= nb; ib++) {
		T[tid][cur][ib] = ib;
		int tmp = ib + abs((na - ia) - (nb - ib));
		if (tmp < min)
			min = tmp;
	}

	if (min > dist)
		return min;

	cur = 1 - cur;

	for (ia = 1; ia <= na; ia++) {
		int ib_st = 0;
		int ib_en = nb;

		ib = 0;
		T[tid][cur][ib] = ia;
		ib_st++;

		min = ia + abs(na - ia - nb + ib);

		for (ib = ib_st; ib <= ib_en; ib++) {
			int ret = oo;

			int d1 = T[tid][1 - cur][ib] + 1;
			int d2 = T[tid][cur][ib - 1] + 1;
			int d3 = T[tid][1 - cur][ib - 1];
			if (a[ia - 1] != b[ib - 1])
				d3++;

			if (d1 < ret)
				ret = d1;
			if (d2 < ret)
				ret = d2;
			if (d3 < ret)
				ret = d3;

			T[tid][cur][ib] = ret;

			/* XXX not tested */
			int difa = na - ia, difb = nb - ib, totalMin = ret
					+ abs(difa - difb);

			if (totalMin < min)
				min = totalMin;
		}

		if (min > dist)
			return min;

		cur = 1 - cur;
	}

	int ret = T[tid][1 - cur][nb];

	return ret;
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
void handleQuery(int tid, int did, DNode_t *cur,int z, int i, int j, char *w, int l,
		int *count)
#else
inline void __attribute__((always_inline)) handleQuery(int tid, int did,
		DNode_t *cur, int z, int type, int i, int j, char *w, int l, int *count)
#endif
{

	if (type == MT_EDIT_DIST) {

		//2nd_lvl_trie_node
		TrieNode_t* trie_node = (TrieNode_t*) cur->data;
		LinkedList_t* _2nd_lvl_list = trie_node->edit_dist_list[z];

		if (!isEmpty(_2nd_lvl_list)) {

			DNode_t* cur2 = _2nd_lvl_list->head.next;

			SegmentData * segData = (SegmentData *) (cur2->data);
			QueryDescriptor * queryData = segData->parentQuery;

			int tmp = queryData->matchDistance;
			queryData->matchDistance = trie_node->max_dist[z];

			int d1;
			if (!(d1 = preCheck(i,
					segData->startIndex - queryData->words[segData->wordIndex],
					queryData->matchDistance))) {
				d1 += editDistance(tid, w, i,
						queryData->words[segData->wordIndex],
						segData->startIndex
								- queryData->words[segData->wordIndex],
						queryData->matchDistance - d1);
			}
			if (d1 <= queryData->matchDistance) {
				int tmp = 0;
				if (!(tmp = preCheck(l - j,
						queryData->words[segData->wordIndex + 1]
								- segData->startIndex - (j - i),
						queryData->matchDistance - d1))) {
					d1 += editDistance(tid, w + j, l - j,
							segData->startIndex + j - i,
							queryData->words[segData->wordIndex + 1]
									- segData->startIndex - (j - i),
							queryData->matchDistance - d1);
				} else
					d1 += tmp;
			}
			queryData->matchDistance = tmp;

			while (cur2 != &(_2nd_lvl_list->tail)) {

				segData = (SegmentData *) (cur2->data);
				queryData = segData->parentQuery;

				if (queryData->docId[tid] != did) {
					queryData->docId[tid] = did;
					queryData->matchedWords[tid] = 0;
				}

				if ((queryData->matchedWords[tid] & (1 << (segData->wordIndex)))
						== 0 && d1 <= queryData->matchDistance) {
					queryData->matchedWords[tid] |= (1 << (segData->wordIndex));

					if (queryData->matchedWords[tid]
							== (1 << (queryData->numWords)) - 1) {
						(*count)++;
					}
				}

				cur2 = cur2->next;
			}

		}
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

void matchWord(int did, int tid, char *w, int l, int *count, Trie_t * trie) {
	if (l > 34)
		return;
	int i = 0;
	for (i = 0; i < l; i++) {
		int j = i;
		TrieNode_t *n = &trie->root;
		while ((n = next_node(n, w[j])) && j < l) {
			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
					&& (i > 0 || n->count[MT_EXACT_MATCH] == 0))
				break;

			j++;
			int en, st, z;
			st = l - 3;
			en = l + 3;
			st = st >= 1 ? st : 1;
			en = en <= 31 ? en : l;
			for (z = st; z <= en + 1; z++) {
				LinkedList_t * list;
				if (z <= en) {
					list = n->list1[z];

					if (!isEmpty(list)) {
						DNode_t *cur = list->head.next;
						while (/*cur->data &&*/cur != &(list->tail)) {

							handleQuery(tid, did, cur, z, MT_EDIT_DIST, i, j, w,
									l, count);

							cur = cur->next;
						}
					}

				} else {
					list = n->list2[l];

					if (!isEmpty(list)) {
						DNode_t *cur = list->head.next;
						while (/*cur->data &&*/cur != &(list->tail)) {

							handleQuery(tid, did, cur, z,
									((SegmentData*) cur->data)->parentQuery->matchType,
									i, j, w, l, count);
							cur = cur->next;
						}
					}

				}

			}
		}
	}
}
