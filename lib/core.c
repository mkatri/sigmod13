/*
 * core.cpp version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Author: Amin Allam
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

//#define CORE_DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <core.h>
#include <unistd.h>
#include "query.h"
#include "trie.h"
#include "word.h"
#include "document.h"
#include "cir_queue.h"
#include "submit_params.h"
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////// DOC THREADING STRUCTS //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

pthread_t threads[NUM_THREADS];
char documents[NUM_THREADS][MAX_DOC_LENGTH];
CircularQueue cirq_free_docs;
CircularQueue cirq_busy_docs;
char *free_docs[NUM_THREADS];
DocumentDescriptor *busy_docs[NUM_THREADS];
pthread_mutex_t docList_lock;
pthread_cond_t docList_avail;
//DynamicArray matches[NUM_THREADS];
int cmpfunc(const QueryID * a, const QueryID * b);
void generate_candidates(char * str, int len, int dist, SegmentData* segData);
///////////////////////////////////////////////////////////////////////////////////////////////
Trie_t *trie;
Trie_t2 * dtrie[NUM_THREADS];
LinkedList_t *docList;
LinkedList_t *queries;
unsigned long docCount;
int cnttt = 0;
Trie3 * eltire;
char lamda = 'a' + 26;
char r[3][3][32][32];
int cntz = 0;
/*QUERY DESCRIPTOR MAP GOES HERE*/
QueryDescriptor qmap[QDESC_MAP_SIZE];
DNode_t qnodes[QDESC_MAP_SIZE];
LinkedList_t * edit_list[QDESC_MAP_SIZE];
//HashTable* ht;
//Trie_t2 *dtrie;
//inline QueryDescriptor * getQueryDescriptor(int queryId) {
//	return qmap[queryId];
//}

inline void addQuery(int queryId, QueryDescriptor * qds) {
	//	qmap[queryId] = qds;
	DNode_t* node = &qnodes[queryId];
	node->data = qds;
	node->prev = queries->tail.prev, node->next = &(queries->tail);
	node->next->prev = node, node->prev->next = node;
//	insert(ht, queryId, node);
}

inline void removeQuery(int queryId, QueryDescriptor *qds) {
	DNode_t* node = &qnodes[queryId];
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

void init() {
	queries = newLinkedList();
	int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	printf("we have %d cores\n", numCPU);
//	ht = new_Hash_Table();
	//THREAD_ENABLE=1;
	trie = newTrie();
	eltire = newTrie3();
//	dtrie = newTrie();
	docList = newLinkedList();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////
int cnt = 0;
void *matcher_thread(void *n) {
	int tid = (uintptr_t) n;
#ifdef THREAD_ENABLE
	while (1) {
#endif
		DocumentDescriptor *doc_desc = (DocumentDescriptor *) cir_queue_remove(
				&cirq_busy_docs);
		char *doc = doc_desc->document;
		int i = 0;
		int matchCount = 0;
		while (doc[i]) {
			while (doc[i] == ' ')
				i++;
			int e = i;
			while (doc[e] != ' ' && doc[e] != '\0')
				e++;
//			puts(doc + e);
			int en, st, z;
			if (!TriewordExist(dtrie[tid], &doc[i], e - i, doc_desc->docId)) {
//				TrieInsert2(dtrie[tid], &doc[i], e - i, doc_desc->docId,tid);
				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
						trie, eltire);

//				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
//						trie2[e - i]);
//				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount);
			} else
				cnt++;
			i = e;
		}

		doc_desc->matches = (QueryID *) malloc(sizeof(QueryID) * matchCount);
		doc_desc->numResults = matchCount;

		/*
		 memcpy(doc_desc->matches, qres, sizeof(QueryID) * matches[tid].tail);
		 qsort(doc_desc->matches, matches[tid].tail, sizeof(QueryID), cmpfunc);
		 */

		i = 0;
		int p = 0;

		DNode_t* cur = queries->head.next;
		while (cur != &(queries->tail)) {
			QueryDescriptor * cqd = (QueryDescriptor *) cur->data;
			if (cqd->docId[tid] == doc_desc->docId
					&& cqd->matchedWords[tid] == (1 << (cqd->numWords)) - 1)
				doc_desc->matches[p++] = cqd->queryId;
			if (p == matchCount)
				break;
			cur = cur->next;
		}

		/*
		 while (i < QDESC_MAP_SIZE) {
		 QueryDescriptor * cqd = &qmap[i++];
		 if (cqd->docId[tid] == doc_desc->docId
		 && cqd->matchedWords[tid] == (1 << (cqd->numWords)) - 1)
		 doc_desc->matches[p++] = cqd->queryId;
		 if (p == matchCount)
		 break;
		 }
		 */
		//XXX could be moved above when we're using array instead of linkedlist
		cir_queue_insert(&cirq_free_docs, doc_desc->document);

		pthread_mutex_lock(&docList_lock);
		append(docList, doc_desc);
		pthread_cond_signal(&docList_avail);
		pthread_mutex_unlock(&docList_lock);
#ifdef THREAD_ENABLE
	}
#endif
	return 0;
}

ErrorCode InitializeIndex() {
	init();
	docCount = 0;
	cir_queue_init(&cirq_free_docs, (void **) &free_docs, NUM_THREADS);
	cir_queue_init(&cirq_busy_docs, (void **) &busy_docs, NUM_THREADS);

	pthread_mutex_init(&docList_lock, NULL);
	pthread_cond_init(&docList_avail, NULL);

	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		free_docs[i] = documents[i];
		//dyn_array_init(&matches[i], RES_POOL_INITSIZE);
		dtrie[i] = newTrie2();
	}
	for (i = 0; i < QDESC_MAP_SIZE; i++)
		edit_list[i] = newLinkedList();

	cirq_free_docs.size = NUM_THREADS;

#ifdef THREAD_ENABLE
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, matcher_thread,
				(void *) (uintptr_t) i);
	}
#endif
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
	printf("%d\n", cntz);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void printWords(char out[6][32], int num) {
#ifdef CORE_DEBUG
	int i = 0;
	for (i = 0; i < num; i++)
	puts(out[i]);
#endif
}

///////////////////////decrease query frequency//////////
inline void optimal_segmentation(char * str, int len, int* start, int t) {
//	char even = 1;

	int i, j, k, ccst, l, m;
	for (i = 0; i <= t; i++)
		start[i] = len;
//	/*EVEN*/
//	k = len - (len / t) * (t);
//	int first = (len + t - 1) / t;
//	int second = len / t;
//
//	start[0] = 0;
//	for (i = 1; i <= k; i++)
//		start[i] = start[i - 1] + first;
//	for (; i < t + 1; i++)
//		start[i] = start[i - 1] + second;
//	/*EVEN*/
//	if(even)return;
	int dp[5][32];
	int choice[5][32];
	for (i = 0; i < len; i++)
		dp[0][i] = 1e6;
	dp[0][len] = 0;
	dp[1][len] = dp[2][len] = dp[3][len] = dp[4][len] = 1e6;
	for (i = 1; i <= t; i++) {
		for (j = 0; j < len; j++) {
			dp[i][j] = 1e6;
			TrieNode_t * ptr = &(trie->root);
			for (k = j; k < len; k++) {
				if (ptr)
					ptr = next_node(ptr, str[k]);
				ccst = (ptr) ?
						ptr->count[0] + ptr->count[1] + ptr->count[2] : 0;
//				ccst = (ptr) ? ptr->counter : 0;
				l = ccst > dp[i - 1][k + 1] ? ccst : dp[i - 1][k + 1];
				m = ccst + dp[i - 1][k + 1];
//				if(str[k]=='f'&&ptr)
//					printf("%d\n",ptr->counter);
				if (l < dp[i][j]) {
					dp[i][j] = l;
					choice[i][j] = k;
				}

			}
		}
	}

//	printf("----->%d\n", dp[t][0]);
//	if (dp[0][t] == 1000000) {
//		for (i = 0; i <= t; putchar('\n'), i++) {
//			for (j = 0; j < len; j++)
//				printf("%8d ", dp[i][j]);
//		}
////		exit(0);
//	}
	int used = 0;
	int index = 0;
	while (index < len) {
		start[used] = index;
		index = choice[t - used][index] + 1;
		used++;
	}
}
/////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {

#ifdef CORE_DEBUG
	printf("query: %d --> %s\n", query_id, query_str);
#endif
//TODO DNode_t ** segmentsData ;
#ifdef THREAD_ENABLE
	waitTillFull(&cirq_free_docs);
#endif
	int in = 0, i = 0, j = 0, wordLength = 0, k, first, second, iq = 0, s;

	int wordSizes[6];
	int numOfWords = 0;
	int numOfSegments = match_dist + 1;
	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = &qmap[query_id];
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;
	queryDescriptor->queryId = query_id;
	for (in = 0; in < NUM_THREADS; in++)
		queryDescriptor->docId[in] = -1;

	addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryDescriptor, query_str, &numOfWords);
	queryDescriptor->numWords = numOfWords;
//	return 0;
	if (match_type == MT_EDIT_DIST) {

		for (in = 0; in < numOfWords; in++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->queryId = query_id;
			sd->wordIndex = in;
			generate_candidates(queryDescriptor->words[in], wordSizes[in],
					match_dist, sd);
		}
		return EC_SUCCESS;
	}

	char segment[32];
	/*initialize the DNode array here*/
	int top = 0;
	queryDescriptor->segmentsData = (DNode_t**) malloc(
			numOfSegments * numOfWords * sizeof(DNode_t *));
	for (top = 0; top < numOfSegments * numOfWords; top++)
		queryDescriptor->segmentsData[top] = 0;
	top = 0;
	//printf("num of words %d\n",numOfWords);

	int segmentStart[6];
	int segLen = 0;
	for (in = 0; in < numOfWords; in++) {
		//get the word length
		iq = 0;
		wordLength = wordSizes[in];

		optimal_segmentation(queryDescriptor->words[in], wordLength,
				segmentStart, numOfSegments);

		for (i = 0; i < numOfSegments; i++) {

			segLen = segmentStart[i + 1] - segmentStart[i];
			queryDescriptor->segmentSizes[in][i] = segLen;
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->startIndex = queryDescriptor->words[in] + segmentStart[i];
			s = iq;
			for (j = 0; j < segLen; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}

			segment[j] = '\0';
			//load the segment data
			sd->queryId = query_id;
			//sd->startIndex = iq - first;
			sd->wordIndex = in;

			//insert in trie
			//			if (match_type == MT_EDIT_DIST) {
			//				queryDescriptor->segmentsData[top++] = TrieInsert(
			//						trie1[wordLength], segment, first, match_type, sd);
			//			} else {
			//				queryDescriptor->segmentsData[top++] = TrieInsert(
			//						trie2[wordLength], segment, first, match_type, sd);
			//			}
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					queryDescriptor->words[in], segLen, match_type, sd,
					wordLength, s, iq);
		}

		//printf("word >> %s\n", queryDescriptor->words[in]);
		//here (wordSizes[in]+1 to add the null at the end of char array

		/*
		 * k here as teste paper mention to get good partition with hamming 1
		 * example : assume word length =10 and distance=3
		 * so we partition the word to 4  segments with length(3,3,2,2)
		 * so first =3;
		 * and second =2;
		 */
		/*how do we prove this*/
//		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
//		first = (wordLength + numOfSegments - 1) / numOfSegments;
//		second = wordLength / numOfSegments;
		// loop on the word to get the segments
//		for (i = 0; i < k; i++) {
//			SegmentData *sd = newSegmentdata();
//			sd->parentQuery = queryDescriptor;
//			sd->startIndex = queryDescriptor->words[in] + iq;
//			for (j = 0; j < first; j++) {
//				segment[j] = *(queryDescriptor->words[in] + iq);
//				iq++;
//			}
//
//			segment[j] = '\0';
//			//load the segment data
//			sd->queryId = query_id;
//			//sd->startIndex = iq - first;
//			sd->wordIndex = in;
//
//			//insert in trie
//			//	printf("segment >>>> %s\n", segment);
////			if (match_type == MT_EDIT_DIST) {
////				queryDescriptor->segmentsData[top++] = TrieInsert(
////						trie1[wordLength], segment, first, match_type, sd);
////			} else {
////				queryDescriptor->segmentsData[top++] = TrieInsert(
////						trie2[wordLength], segment, first, match_type, sd);
////			}
//			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
//					first, match_type, sd, wordLength);
//		}
//
//		// loop on the word to get the segments
//		for (i = 0; (i < numOfSegments - k) && second; i++) {
//			SegmentData *sd = newSegmentdata();
//			sd->parentQuery = queryDescriptor;
//			sd->startIndex = queryDescriptor->words[in] + iq;
//			for (j = 0; j < second; j++) {
//				segment[j] = *(queryDescriptor->words[in] + iq);
//				iq++;
//			}
//			segment[j] = '\0';
//			//load segments data
//			sd->queryId = query_id;
//			//sd->startIndex = iq - second;
//			sd->wordIndex = in;
//			//insert in trie
//			//	printf("segment >>>> %s\n", segment);
////			if (match_type == MT_EDIT_DIST) {
////				queryDescriptor->segmentsData[top++] = TrieInsert(
////						trie1[wordLength], segment, second, match_type, sd);
////			} else {
////				queryDescriptor->segmentsData[top++] = TrieInsert(
////						trie2[wordLength], segment, second, match_type, sd);
////			}
//			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
//					second, match_type, sd, wordLength);
//		}
	}

	return EC_SUCCESS;

}

/*
 * this method take string and split it to subStrings(words) as the words are space separated
 */

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx) {

	int iq = 0;
	char *output = desc->queryString;
	char **words = desc->words;

	*idx = 0;
	int idx2 = 0;

	// if the string is null
	//TODO i think we must fire error
	if (!query_str[iq])
		return;
	// assume there are spaces in the first
	while (query_str[iq] && query_str[iq] == ' ')
		iq++;

	words[*idx] = output;
	int idx1 = 0;
	// loop and get the words
	while (query_str[iq]) {

		if (query_str[iq] == ' ') {
			while (query_str[iq] == ' ')
				iq++;
			if (query_str[iq]) {
				length[(*idx)] = idx1;
				(*idx)++;
				words[*idx] = &output[idx2];
				idx1 = 0;
			}

		}

		//to handle spaces in the end of query
		if (query_str[iq]) {
			output[idx2++] = query_str[iq];
			idx1++;
			iq++;
		}
	}

	length[(*idx)] = idx1;
	(*idx)++;
	words[*idx] = &output[idx2];
}

///////////////////////////////////////////////////////////////////////////////////////////////

int cnt5 = 0;
ErrorCode EndQuery(QueryID query_id) {
#ifdef CORE_DEBUG
	puts("inside here");
#endif
#ifdef THREAD_ENABLE
	waitTillFull(&cirq_free_docs);
#endif

	QueryDescriptor* queryDescriptor = &qmap[query_id];

	if (queryDescriptor->matchType == MT_EDIT_DIST) {
		LinkedList_t * list = edit_list[query_id];
		DNode_t *cur = list->head.next;
		while (cur != &(list->tail)) {
			DNode_t * node = (DNode_t *) cur->data;
			delete_node(node);
			cur = cur->next;
		}
		return EC_SUCCESS;
	}

	int i, j;
	int in, iq, wordLength, numOfSegments = queryDescriptor->matchDistance + 1,
			k, first, second;
	char segment[32];
	int top = 0;
	int segmentStart[6];
	int segLen = 0;

	for (in = 0; in < 5 && queryDescriptor->words[in + 1]; in++) {

		//get the word length
		iq = 0;
		wordLength = queryDescriptor->words[in + 1]
				- queryDescriptor->words[in];

		for (i = 0; i < numOfSegments; i++) {

			segLen = queryDescriptor->segmentSizes[in][i];

			for (j = 0; j < segLen; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}

			segment[j] = '\0';

			DNode_t* node = queryDescriptor->segmentsData[top++];
			SegmentData* sd = (SegmentData *) node->data;

			if (sd->parentQuery->matchType == MT_EDIT_DIST
					&& node->next->data == 0 && node->prev->data == 0) {
				delete_node(node->tmp);
			}

			delete_node(node);

			TrieDelete(trie, segment, segLen, queryDescriptor->matchType);
		}

		//get the word length
		//here (wordSizes[in]+1 to add the null at the end of char array
		/*
		 * k here as teste paper mention to get good partition with hamming 1
		 * example : assume word length =10 and distance=3
		 * so we partition the word to 4  segments with length(3,3,2,2)
		 * so first =3;
		 * and second =2;
		 */
		/*how do we prove this*/
#ifdef CORE_DEBUG
		printf(">>>>>     %d %d\n", wordLength, numOfSegments);
#endif

//		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
//		first = (wordLength + numOfSegments - 1) / numOfSegments;
//		second = wordLength / numOfSegments;
//		// loop on the word to get the segments
//		for (i = 0; i < k; i++) {
//			for (j = 0; j < first; j++) {
//				segment[j] = queryDescriptor->words[in][iq];
//				iq++;
//			}
//			segment[j] = '\0';
//
//			//Delete from the linked list in trie nodes
//			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
//			//Delete from the trie
////			if (queryDescriptor->matchType == MT_EDIT_DIST) {
////				TrieDelete(trie1[wordLength], segment, first,
////						queryDescriptor->matchType);
////			} else {
////				TrieDelete(trie2[wordLength], segment, first,
////						queryDescriptor->matchType);
////			}
//			TrieDelete(trie, segment, first, queryDescriptor->matchType);
//
//		}
//
//		// loop on the word to get the segments
//		for (i = 0; (i < numOfSegments - k) && second; i++) {
//			for (j = 0; j < second; j++) {
//				segment[j] = queryDescriptor->words[in][iq];
//				iq++;
//			}
//			segment[j] = '\0';
//
//			//Delete from the linked list in trie nodes
//			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
//			//Delete from the trie
////			if (queryDescriptor->matchType == MT_EDIT_DIST) {
////				TrieDelete(trie1[wordLength], segment, second,
////						queryDescriptor->matchType);
////			} else {
////				TrieDelete(trie2[wordLength], segment, second,
////						queryDescriptor->matchType);
////			}
//			TrieDelete(trie, segment, second, queryDescriptor->matchType);
//
//		}
//	}
////	freeQueryDescriptor(queryDescriptor);
////	delete_H(ht, query_id);
////	node = (DNode_t*) get(ht, query_id);
////	qmap[query_id] = 0;

	}
	removeQuery(query_id, queryDescriptor);
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////
int cmpfunc(const QueryID * a, const QueryID * b) {
	return (*a - *b);
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	docCount++;
	char *doc_buf = (char *) cir_queue_remove(&cirq_free_docs);
	strcpy(doc_buf, doc_str);
	DocumentDescriptor *desc = (DocumentDescriptor *) malloc(
			sizeof(DocumentDescriptor));
	desc->docId = doc_id;
	desc->document = doc_buf;
	cir_queue_insert(&cirq_busy_docs, desc);
#ifndef THREAD_ENABLE
	matcher_thread(1);
#endif
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	if (docCount == 0)
		return EC_NO_AVAIL_RES;
	pthread_mutex_lock(&docList_lock);
	while (isEmpty(docList))
		pthread_cond_wait(&docList_avail, &docList_lock);
	DNode_t *node = docList->head.next;
	DocumentDescriptor* doc_desc = (DocumentDescriptor *) (node->data);
	delete_node(node);
	pthread_mutex_unlock(&docList_lock);

	docCount--;
	*p_query_ids = doc_desc->matches;
	*p_doc_id = doc_desc->docId;
	*p_num_res = doc_desc->numResults;
	if (doc_desc->numResults == 0)
		free(doc_desc->matches);
	free(doc_desc);
	return EC_SUCCESS;
}
char result[300000][33];
int lengths[300000];
int maxLen=0;
void generate_candidates(char * str, int len, int dist, SegmentData* segData) {
	int start = 0, end = 1;
	lengths[0] = len;
	int i;
	for (i = 0; i < len; i++)
		result[0][i] = str[i];
	int id = 0;
	int resIndex = 1;
	while (dist > 0) {
		for (id = start; id < end; id++) {
			str = result[id];
			int i, j;
			// insert
			for (i = 0; i <= lengths[id]; i++) {
				int ptr = 0;
				for (j = 0; j < i; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][ptr++] = lamda;
				for (j = i; j < lengths[id]; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][ptr] = '\0';
				lengths[resIndex] = lengths[id] + 1;
				resIndex++;
			}
			// delete
			for (i = 0; i < lengths[id]; i++) {
				int ptr = 0;
				for (j = 0; j < i; j++)
					result[resIndex][ptr++] = str[j];
				for (j = i + 1; j < lengths[id]; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][ptr] = '\0';
				lengths[resIndex] = lengths[id] - 1;
				resIndex++;
			}
			// swap
			for (i = 0; i < lengths[id]; i++) {
				int ptr = 0;
				for (j = 0; j < lengths[id]; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][i] = lamda;
				result[resIndex][ptr] = '\0';
				lengths[resIndex] = lengths[id];
				resIndex++;
			}
		}
		start = end;
		end = resIndex;
		dist--;
	}
//	int ii = 0;
//	printf("-----------------\n");
//	for (ii = 0; ii < resIndex; ii++)
//		printf("ssss: %s\n", result[ii]);

//	printf("-----------------\n");
	int ind = start;
	while (ind < end) {
		DNode_t * node = InsertTrie3(eltire, result[ind], lengths[ind],
				segData);
//		printf("string = %s\n",result[ind]);
		append(edit_list[segData->queryId], node);
		ind++;
	}
}
//void generate_candidates(char * str, int len, int dist, SegmentData* segData) {
//	if (len == 0)
//		return;
//	cntz++;
//	if (dist == 0) {
////		printf("stringaya =====  %s \n", str);
//		DNode_t * node = InsertTrie3(eltire, str, len, segData);
//		append(edit_list[segData->queryId], node);
//		// insert in trie;
//		return;
//	}
//
//	int i, j;
//	// insert
//	for (i = 0; i <= len; i++) {
//		for (j = 0; j < i; j++)
//			r[dist - 1][0][i][j] = str[j];
//		r[dist - 1][0][i][i] = lamda;
//		for (j = i; j < len; j++)
//			r[dist - 1][0][i][j + 1] = str[j];
//	}
//	// delete
//	for (i = 0; i < len; i++) {
//		for (j = 0; j < i; j++)
//			r[dist - 1][1][i][j] = str[j];
//		for (j = i + 1; j < len; j++)
//			r[dist - 1][1][i][j - 1] = str[j];
//	}
//	// swap
//	for (i = 0; i < len; i++) {
//		for (j = 0; j < len; j++)
//			r[dist - 1][2][i][j] = str[j];
//		r[dist - 1][2][i][i] = lamda;
//	}
//
//	for (i = 0; i < len; i++) {
//		generate_candidates(r[dist - 1][0][i], len + 1, dist - 1, segData);
//		generate_candidates(r[dist - 1][1][i], len - 1, dist - 1, segData);
//		generate_candidates(r[dist - 1][2][i], len, dist - 1, segData);
//	}
//	generate_candidates(r[dist - 1][0][len], len + 1, dist - 1, segData);
//}
///////////////////////////////////////////
void core_test() {
	InitializeIndex();
	char f[32] = "air";
	char f2[32] = "aix";

	StartQuery(5, f, MT_EDIT_DIST, 1);
//	StartQuery(75, f2, MT_EDIT_DIST, 1);
	//	EndQuery(7);
//		dfs(&(trie->root));
	//	printf("done\n");

	//hashTest();
	MatchDocument(10, "air");
	//	MatchDocument(11, "ok no fucker");
	//	MatchDocument(20, "fuck you oknofutcher");
	//	MatchDocument(30, "fuck mother you oknofucker father");
	DocID did;
	QueryID *qid;
	unsigned int numRes;
	GetNextAvailRes(&did, &numRes, &qid);
	int i;
	for (i = 0; i < numRes; i++)
		printf("---->%d\n", qid[i]);
	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
//	GetNextAvailRes(&did, &numRes, &qid);
}
