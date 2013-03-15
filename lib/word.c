#include <pthread.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "dyn_array.h"

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

	static int T[NUM_THREADS][2][100];

	int ia, ib;

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

void matchWord(int did, int tid, char *w, int l, int *count) {
	QueryDescriptor head;
	QueryDescriptor tail;
	/*
	head.next[tid] = &tail, tail.prev[tid] = &head;
	head.prev[tid] = 0, tail.next[tid] = 0;
	*/
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
					/*XXX somewhere you set the data of the queryData->docWord = list tail, this is not cool*/
					SegmentData * segData = (SegmentData *) (cur->data);
					QueryDescriptor * queryData = segData->parentQuery;
					int type = queryData->matchType;

					if (queryData->docId[tid] != did) {
						queryData->docId[tid] = did;
						//queryData->docWord[tid] = 0;
						queryData->matchedWords[tid] = 0;
					}

					if (((queryData->matchedWords[tid])
							& (1 << (segData->wordIndex)))) {
						//	|| (queryData->docWord == w	&& queryData->cur_dist[x] > queryData->matchDistance)
						//if already matched or if already exceeded matchDistance
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
						}
					} else if (type == MT_HAMMING_DIST) {
						/*
						if (i
								== segData->startIndex
										- queryData->words[segData->wordIndex]
								&& (l - j)
										== queryData->words[segData->wordIndex
												+ 1] - segData->startIndex
												- (j - i)) {
							if (queryData->next_seg[tid][segData->wordIndex]
									< segData->segmentIndex) {
								SegmentData *begin_match =
										&queryData->segmentsData[segData->wordIndex][queryData->next_seg[tid][segData->wordIndex]];
								queryData->cur_dist[tid][segData->wordIndex] +=
										hammingDistance(
												w
														+ (begin_match->startIndex
																- queryData->words[segData->wordIndex]),
												begin_match->startIndex,
												segData->startIndex
														- begin_match->startIndex,
												queryData->matchDistance
														- queryData->cur_dist[tid][segData->wordIndex]);

							}

							queryData->next_seg[tid][segData->wordIndex] =
									segData->segmentIndex + 1;

							queryData->part_matchedWords[tid] |= 1
									<< segData->wordIndex;

							if (queryData->docWord[tid] != w
									&& queryData->cur_dist[tid][segData->wordIndex]
											<= queryData->matchDistance) {
								queryData->docWord[tid] = w;
								queryData->prev[tid] = tail.prev[tid];
								queryData->next[tid] = &tail;
								queryData->next[tid]->prev[tid] = queryData;
								queryData->prev[tid]->next[tid] = queryData;
							}
						}
						*/
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
	}

	/*
	QueryDescriptor *curr = head.next[tid];
	while (curr != &tail) {
		if (curr->queryId == 32) {

			printf("here's the problem\n");
		}
		int i;
		for (i = 0; i < curr->numWords; i++) {
			if ((curr->part_matchedWords[tid] & (1 << i)) != 0) {
				if (curr->cur_dist[tid][i] <= curr->matchDistance) {
					if (curr->next_seg[tid][i] == curr->matchDistance + 1) {
						curr->matchedWords[tid] |= (1 << i);
						if (curr->matchedWords[tid]
								== (1 << (curr->numWords)) - 1) {
							(*count)++;
						}
					} else {
						SegmentData *begin_match =
								&curr->segmentsData[i][curr->next_seg[tid][i]];
						curr->cur_dist[tid][i] += hammingDistance(
								w + (begin_match->startIndex - curr->words[i]),
								begin_match->startIndex,
								curr->words[i + 1] - begin_match->startIndex,
								curr->matchDistance - curr->cur_dist[tid][i]);
						if (curr->cur_dist[tid][i] <= curr->matchDistance) {
							curr->matchedWords[tid] |= (1 << i);
							if (curr->matchedWords[tid]
									== (1 << (curr->numWords)) - 1) {
								(*count)++;
							}
						}
					}
				}

				curr->next_seg[tid][i] = 0;
				curr->cur_dist[tid][i] = 0;
			}
		}

		curr->part_matchedWords[tid] = 0;
		curr = curr->next[tid];
	}
	*/
	/*TODO reset query descriptors */
}
