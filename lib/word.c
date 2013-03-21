#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include <core.h>

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
//	if (!strncmp(w, "rap", 3)) {
//		printf("%d\n", doc_id);
//		fflush(0);
//		puts(w);
//		puts("=====");
//	}
	long long* tmp = el_Fashee5_fel_address;

	if (el_Fashee5_fel_address) {
		printf("%lld\n", el_Fashee5_fel_address);
//		el_Fashee5_fel_address = 0;
	}
	int i = 0;
	for (i = 0; i < l; i++) {
		int j = i;
		TrieNode_t *n = &(trie->root);

		while (j < l && (n = next_node(n, w[j]))) {
			if (j > 0 && w[j - 1] == 'r' && w[j] == 'a' && w[j + 1] == 'p') {

				printf("==> %d %d %lld\n", i, doc_id, n->next['p' - 'a']);
				fflush(0);
			}
//			printf("==> %d\n", doc_id, w);fflush(0);
			if (n->count[MT_EDIT_DIST] == 0 && n->count[MT_HAMMING_DIST] == 0
					&& i > 0)
				break;
			j++;

			if (n->isTerminal) {

				DNode_t *cur = n->partsNodesList->head.next;

				while (cur != &(n->partsNodesList->tail)) { //each cur is group of equal segments (not equal in match parameters)

					TrieNode_t* node = (TrieNode_t*) cur->data; //all with equal segment strings
					DNode_t* segmentsItr = node->SegmentDataList->head.next;
					int minHammingDist[2];
					minHammingDist[0] = minHammingDist[1] = 50;
					int minEditDist[2];
					minEditDist[0] = minEditDist[1] = 50;
					char ok;

					while (segmentsItr != &(node->SegmentDataList->tail)) { //each one maybe with different match parameters

						ok = 0;
						partsNode* partData = (partsNode *) (segmentsItr->data);
						SegmentData * segData =
								(SegmentData*) partData->queryData;
						QueryDescriptor * queryData = segData->parentQuery;
						int type = queryData->matchType;

						if (queryData->docId != doc_id) {
							queryData->docId = doc_id;
							queryData->matchedWords = 0;
						}

						if (segData->docId != doc_id) {
							segData->leftMatched = 0;
							segData->rightMatched = 0;
							segData->reminderDistance =
									queryData->matchDistance;
							segData->docId = doc_id;
						}

						if (type == MT_HAMMING_DIST) {
							if (segData->reminderDistance
									> minHammingDist[partData->isRight]) {
								segData->reminderDistance -=
										minHammingDist[partData->isRight];
								ok = 1;
							} else if (minHammingDist[partData->isRight]
									== 50) {

								if (!partData->isRight) { //left
									if (i == partData->len) {
										minHammingDist[partData->isRight] =
												hammingDistance(
														partData->startChar, w,
														i, 3);
									} else {
										minHammingDist[partData->isRight] = 60;
									}
								} else { //right

									if (l - j == partData->len) {
										minHammingDist[partData->isRight] =
												hammingDistance(
														partData->startChar,
														w + j, l - j, 3);
									} else {
										minHammingDist[partData->isRight] = 60;
									}
								}

								if (segData->reminderDistance
										> minHammingDist[partData->isRight]) {
									segData->reminderDistance -=
											minHammingDist[partData->isRight];
									ok = 1;
								}

							}

						} else if (type == MT_EDIT_DIST) {

							if (minEditDist[partData->isRight]
									<= segData->reminderDistance) {
								ok = 1;
								segData->reminderDistance -=
										minEditDist[partData->isRight];
							} else if (minEditDist[partData->isRight] == 50) {
								if (!partData->isRight) { //left
									minEditDist[partData->isRight] =
											editDistance(w, i,
													partData->startChar,
													partData->len, 3);
								} else {
									minEditDist[partData->isRight] =
											editDistance(w + j, l - j,
													partData->startChar,
													partData->len, 3);
								}

								if (minEditDist[partData->isRight]
										<= segData->reminderDistance) {
									ok = 1;
									segData->reminderDistance -=
											minEditDist[partData->isRight];
								}

							}

						} else if (/*type == MT_EXACT_MATCH &&*/i == 0
								&& j == l) { // Exact matching must be done from the start of the word only
							ok = 1;
							segData->rightMatched = segData->leftMatched = 1;
						}

						if (ok) {
							if (segData->rightMatched && segData->leftMatched) {
								queryData->matchedWords |= (1
										<< (segData->wordIndex));
								if (queryData->matchedWords
										== (1 << (queryData->numWords)) - 1) {
									(*count)++;
									if (pos == sizeOfPool)
										doubleSize();
									qres[pos++] = queryData->queryId;
								}
							} else if (partData->isRight) {
								segData->rightMatched = 1;
							} else {
								segData->leftMatched = 1;
							}
						}
						segmentsItr = segmentsItr->next;
					}

					cur = cur->next;
				}
			}
		}
	}
}
