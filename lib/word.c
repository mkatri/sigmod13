#include <pthread.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "Edit_Distance.h"
#include "word.h"

//Time=2652[2s:652ms]

extern Edit_Distance* ed[];
extern Trie_t *trie;

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
	int tmp;

	if ((tmp = get_editDistance(tid, a, na, b, nb, ed[tid])) > -1)
		return tmp;

	static int T[NUM_THREADS][2][100];

	ED_Trie_Node* node = ed[tid]->trie->root;

	int ia, ib;

	int cur = 0;
	ia = 0;

	for (ib = 0; ib <= nb; ib++)
		T[tid][cur][ib] = ib;

	cur = 1 - cur;

	for (ia = 1; ia <= na; ia++) {
		int ib_st = 0;
		int ib_en = nb;

		ib = 0;
		T[tid][cur][ib] = ia;
		ib_st++;

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

		}

		node = add_editDistance(a, ia, b, nb, T[tid][cur], ed[tid], node);
		cur = 1 - cur;
	}

	currNode[tid] = node;
	int ret = T[tid][1 - cur][nb];

	return ret;
}

void matchWord(int did, int tid, char *w, int l, int *count) {
	if (l > 35)
		return;

	int i = 0;
	for (i = 0; i < l; i++) {
		int j = i;
		TrieNode_t *n = &trie->root;
		while ((n = next_node(n, w[j])) && j < l) {

			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
					&& i > 0)
				break;

			j++;
			if (!isEmpty(n->list)) {
				DNode_t *cur = n->list->head.next;
				while (cur->data && cur != &(n->list->tail)) {

					SegmentData * segData = (SegmentData *) (cur->data);
					QueryDescriptor * queryData = segData->parentQuery;
					int type = queryData->matchType;

					if (queryData->docId[tid] != did) {
						queryData->docId[tid] = did;
						queryData->matchedWords[tid] = 0;
					}

					if (((queryData->matchedWords[tid])
							& (1 << (segData->wordIndex)))) {
						cur = cur->next;
						continue;
					}

					if (type == MT_EDIT_DIST) {

						int d1;
						if ((d1 = preCheck(i,
								segData->startIndex
										- queryData->words[segData->wordIndex],
								queryData->matchDistance))
								<= queryData->matchDistance) {

							d1 +=
									editDistance(tid, w, i,
											queryData->words[segData->wordIndex],
											segData->startIndex
													- queryData->words[segData->wordIndex],
											queryData->matchDistance - d1);

							ED_Trie_Node* tmpnode = currNode[tid];
							currNode[tid] = 0;

							if (d1 <= queryData->matchDistance) {
								d1 += editDistance(tid, w + j, l - j,
										segData->startIndex + j - i,
										queryData->words[segData->wordIndex + 1]
												- segData->startIndex - (j - i),
										queryData->matchDistance - d1);
							}
							if (d1 <= queryData->matchDistance) {
								queryData->matchedWords[tid] |= (1
										<< (segData->wordIndex));

								if (queryData->matchedWords[tid]
										== (1 << (queryData->numWords)) - 1) {
									(*count)++;
								}
							}
							currNode[tid] = tmpnode;
						}
					} else if (type == MT_HAMMING_DIST) {
						if (i
								== segData->startIndex
										- queryData->words[segData->wordIndex]
								&& (l - j)
										== queryData->words[segData->wordIndex
												+ 1] - segData->startIndex
												- (j - i)) {
							int d1 = hammingDistance(w,
									queryData->words[segData->wordIndex], i,
									queryData->matchDistance);
							if (d1 <= queryData->matchDistance) {
								d1 += hammingDistance(w + j,
										queryData->words[segData->wordIndex]
												+ j, l - j,
										queryData->matchDistance - d1);

								if (d1 <= queryData->matchDistance) {
									queryData->matchedWords[tid] |= (1
											<< (segData->wordIndex));

									if (queryData->matchedWords[tid]
											== (1 << (queryData->numWords))
													- 1) {
										(*count)++;
									}
								}
							}
						}
					} else if (i == 0 && j == l) { // Exact matching must be done from the start of the word only
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
		currNode[tid] = 0;
	}
}
