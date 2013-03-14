#include "linked_list.h"
#include "trie.h"
#include "query.h"

extern Trie_t *trie;
extern int pos;
extern int * qres;
extern int sizeOfPool;

void doubleSize() {
	sizeOfPool <<= 1;
	qres = (int*) realloc(qres, sizeof(int) * sizeOfPool);
}

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

int editDistance(char* a, int na, char* b, int nb, int dist) {
	int oo = 0x7FFFFFFF;

	static int T[2][100];

	int ia, ib;

	int cur = 0, min;
	ia = 0;
	min = 1 << 30;

	for (ib = 0; ib <= nb; ib++) {
		T[cur][ib] = ib;
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
		T[cur][ib] = ia;
		ib_st++;

		min = ia + abs(na - ia - nb + ib);

		for (ib = ib_st; ib <= ib_en; ib++) {
			int ret = oo;

			int d1 = T[1 - cur][ib] + 1;
			int d2 = T[cur][ib - 1] + 1;
			int d3 = T[1 - cur][ib - 1];
			if (a[ia - 1] != b[ib - 1])
				d3++;

			if (d1 < ret)
				ret = d1;
			if (d2 < ret)
				ret = d2;
			if (d3 < ret)
				ret = d3;

			T[cur][ib] = ret;

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

	int ret = T[1 - cur][nb];

	return ret;
}

void matchWord(char *w, int l, int *count, int doc_id) {

	if (l > 35)
		return;

	int i = 0;
	for (i = 0; i < l; i++) {
		int j = i;
		TrieNode_t *n = trie;
//		TrieNode_t *p = 0;
		while ((n = next_node(n, w[j])) && j < l) {
			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
					&& i > 0)
				break;
			j++;
			if (!isEmpty(n->list)) {
				DNode_t *cur = n->list->head.next;
				while (cur->data && cur != &(n->list->tail)) {
					/*XXX somewhere you set the data of the list tail, this is not cool*/
					SegmentData * segData = (SegmentData *) (cur->data);
					QueryDescriptor * queryData = segData->parentQuery;
					int type = queryData->matchType;
					if (queryData->docId != doc_id) {
						queryData->docId = doc_id;
						queryData->matchedWords = 0;
					}
					if (((queryData->matchedWords) & (1 << (segData->wordIndex)))) {
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
									editDistance(w, i,
											queryData->words[segData->wordIndex],
											segData->startIndex
													- queryData->words[segData->wordIndex],
											queryData->matchDistance - d1);
							if (d1 <= queryData->matchDistance) {
								d1 += editDistance(w + j, l - j,
										segData->startIndex + j - i,
										queryData->words[segData->wordIndex + 1]
												- segData->startIndex - (j - i),
										queryData->matchDistance - d1);

								if (d1 <= queryData->matchDistance) {
									queryData->matchedWords |= (1
											<< (segData->wordIndex));
									if (queryData->matchedWords
											== (1 << (queryData->numWords))
													- 1) {
										(*count)++;
										if (pos == sizeOfPool)
											doubleSize();
										qres[pos++] = queryData->queryId;
									}
								}
							}
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
									queryData->matchedWords |= (1
											<< (segData->wordIndex));
									if (queryData->matchedWords
											== (1 << (queryData->numWords))
													- 1) {
										(*count)++;
										if (pos == sizeOfPool)
											doubleSize();
										qres[pos++] = queryData->queryId;

									}
								}
							}
						}
					} else if (i == 0 && j == l) { // Exact matching must be done from the start of the word only
						queryData->matchedWords |= (1 << (segData->wordIndex));
						if (queryData->matchedWords
								== (1 << (queryData->numWords)) - 1) {
							(*count)++;
							if (pos == sizeOfPool)
								doubleSize();
							qres[pos++] = queryData->queryId;
						}
					}
					cur = cur->next;
				}
			}
//			p = n;
		}
	}
}
