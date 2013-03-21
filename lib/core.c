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
#include <core.h>
#include "query.h"
#include "trie.h"
#include "document.h"
#include "Hash_Table.h"
#include "cir_queue.h"
#include "submit_params.h"
#include "dyn_array.h"
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
///////////////////////////////////////////////////////////////////////////////////////////////
Trie_t *trie;
Trie_t * dtrie[NUM_THREADS];
LinkedList_t *docList;
//LinkedList_t *queries;
unsigned long docCount;

/*QUERY DESCRIPTOR MAP GOES HERE*/
#define QDESC_MAP_SIZE 20000
QueryDescriptor qmap[QDESC_MAP_SIZE];
//HashTable* ht;
//Trie_t2 *dtrie;
//inline QueryDescriptor * getQueryDescriptor(int queryId) {
//	return qmap[queryId];
//}
/*
 inline void addQuery(int queryId, QueryDescriptor * qds) {
 //	qmap[queryId] = qds;

 DNode_t* node = append(queries, qds);
 insert(ht, queryId, node);

 }
 */
/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

void init() {
//	queries = newLinkedList();
//	ht = new_Hash_Table();
	trie = newTrie();
//	dtrie = newTrie();
	docList = newLinkedList();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////
int cnt = 0;
void *matcher_thread(void *n) {
	int tid = n;
#ifdef THREAD_ENABLE
	while (1) {
#endif
		DocumentDescriptor *doc_desc = cir_queue_remove(&cirq_busy_docs);
		char *doc = doc_desc->document;
		int i = 0;
		int matchCount = 0;
		while (doc[i]) {
			while (doc[i] == ' ')
				i++;
			int e = i;
			while (doc[e] != ' ' && doc[e] != '\0')
				e++;
			int en, st, z;
			if (!TriewordExist(dtrie[tid], &doc[i], e - i, doc_desc->docId)) {
//				TrieInsert2(dtrie[tid], &doc[i], e - i, doc_desc->docId,tid);
				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
						trie);
//				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
//						trie2[e - i]);
//				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount);
			} else
				cnt++;
			i = e;
		}

		doc_desc->matches = malloc(sizeof(QueryID) * matchCount);
		doc_desc->numResults = matchCount;

		/*
		 memcpy(doc_desc->matches, qres, sizeof(QueryID) * matches[tid].tail);
		 qsort(doc_desc->matches, matches[tid].tail, sizeof(QueryID), cmpfunc);
		 */

		i = 0;
		int p = 0;
		/*
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
		 */
		while (i < QDESC_MAP_SIZE) {
			QueryDescriptor * cqd = &qmap[i++];
			if (cqd->docId[tid] == doc_desc->docId
					&& cqd->matchedWords[tid] == (1 << (cqd->numWords)) - 1)
				doc_desc->matches[p++] = cqd->queryId;
			if (p == matchCount)
				break;
		}
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
	cir_queue_init(&cirq_free_docs, &free_docs, NUM_THREADS);
	cir_queue_init(&cirq_busy_docs, &busy_docs, NUM_THREADS);

	pthread_mutex_init(&docList_lock, NULL);
	pthread_cond_init(&docList_avail, NULL);

	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		free_docs[i] = documents[i];
		//dyn_array_init(&matches[i], RES_POOL_INITSIZE);
		dtrie[i] = newTrie();
	}
	cirq_free_docs.size = NUM_THREADS;

#ifdef THREAD_ENABLE
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, matcher_thread, i);
	}
#endif
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
	printf("%d\n", cnt);
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

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {
//#ifdef CORE_DEBUG
//	printf("query: %d --> %s\n", query_id, query_str);
//#endif

//TODO DNode_t ** segmentsData ;
	waitTillFull(&cirq_free_docs);
	int in = 0, i = 0, j = 0, wordLength = 0, k, first, second, iq = 0;

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

	//addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryDescriptor, query_str, &numOfWords);
	queryDescriptor->numWords = numOfWords;
//	return 0;

	char segment[32];
	/*initialize the DNode array here*/
	int top = 0;
	queryDescriptor->segmentsData = (DNode_t**) malloc(
			numOfSegments * numOfWords * sizeof(DNode_t *));
	for (top = 0; top < numOfSegments * numOfWords; top++)
		queryDescriptor->segmentsData[top] = 0;
	top = 0;
	//printf("num of words %d\n",numOfWords);
	for (in = 0; in < numOfWords; in++) {
		//get the word length
		iq = 0;
		wordLength = wordSizes[in];
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
		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
		first = (wordLength + numOfSegments - 1) / numOfSegments;
		second = wordLength / numOfSegments;
		// loop on the word to get the segments
		for (i = 0; i < k; i++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->startIndex = queryDescriptor->words[in] + iq;
			for (j = 0; j < first; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}

			segment[j] = '\0';
			//load the segment data
			sd->queryId = query_id;
			//sd->startIndex = iq - first;
			sd->wordIndex = in;

			//insert in trie
			//	printf("segment >>>> %s\n", segment);
//			if (match_type == MT_EDIT_DIST) {
//				queryDescriptor->segmentsData[top++] = TrieInsert(
//						trie1[wordLength], segment, first, match_type, sd);
//			} else {
//				queryDescriptor->segmentsData[top++] = TrieInsert(
//						trie2[wordLength], segment, first, match_type, sd);
//			}
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					first, match_type, sd, wordLength);
		}

		// loop on the word to get the segments
		for (i = 0; (i < numOfSegments - k) && second; i++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->startIndex = queryDescriptor->words[in] + iq;
			for (j = 0; j < second; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}
			segment[j] = '\0';
			//load segments data
			sd->queryId = query_id;
			//sd->startIndex = iq - second;
			sd->wordIndex = in;
			//insert in trie
			//	printf("segment >>>> %s\n", segment);
//			if (match_type == MT_EDIT_DIST) {
//				queryDescriptor->segmentsData[top++] = TrieInsert(
//						trie1[wordLength], segment, second, match_type, sd);
//			} else {
//				queryDescriptor->segmentsData[top++] = TrieInsert(
//						trie2[wordLength], segment, second, match_type, sd);
//			}
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					second, match_type, sd, wordLength);
		}
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

ErrorCode EndQuery(QueryID query_id) {
#ifdef CORE_DEBUG
	puts("inside here");
#endif

//	QueryDescriptor* queryDescriptor = getQueryDescriptor(query_id);
	waitTillFull(&cirq_free_docs);
	QueryDescriptor* queryDescriptor = &qmap[query_id];
	int i, j;
	int in, iq, wordLength, numOfSegments = queryDescriptor->matchDistance + 1,
			k, first, second;
	char segment[32];
	int top = 0;
	for (in = 0; in < 5 && queryDescriptor->words[in + 1]; in++) {

		//get the word length
		iq = 0;
		wordLength = queryDescriptor->words[in + 1]
				- queryDescriptor->words[in];
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

		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
		first = (wordLength + numOfSegments - 1) / numOfSegments;
		second = wordLength / numOfSegments;
		// loop on the word to get the segments
		for (i = 0; i < k; i++) {
			for (j = 0; j < first; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
			//Delete from the trie
//			if (queryDescriptor->matchType == MT_EDIT_DIST) {
//				TrieDelete(trie1[wordLength], segment, first,
//						queryDescriptor->matchType);
//			} else {
//				TrieDelete(trie2[wordLength], segment, first,
//						queryDescriptor->matchType);
//			}
			TrieDelete(trie, segment, first, queryDescriptor->matchType);

		}

		// loop on the word to get the segments
		for (i = 0; (i < numOfSegments - k) && second; i++) {
			for (j = 0; j < second; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
			//Delete from the trie
//			if (queryDescriptor->matchType == MT_EDIT_DIST) {
//				TrieDelete(trie1[wordLength], segment, second,
//						queryDescriptor->matchType);
//			} else {
//				TrieDelete(trie2[wordLength], segment, second,
//						queryDescriptor->matchType);
//			}
			TrieDelete(trie, segment, second, queryDescriptor->matchType);

		}
	}
//	freeQueryDescriptor(queryDescriptor);
//	delete_H(ht, query_id);
//	node = (DNode_t*) get(ht, query_id);
//	qmap[query_id] = 0;
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int cmpfunc(const QueryID * a, const QueryID * b) {
	return (*a - *b);
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	docCount++;
	char *doc_buf = cir_queue_remove(&cirq_free_docs);
	strcpy(doc_buf, doc_str);
	DocumentDescriptor *desc = malloc(sizeof(DocumentDescriptor));
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
	delete(node);
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

///////////////////////////////////////////
void core_test() {
	InitializeIndex();
	char f[32] = " ecookr  ";

	StartQuery(5, f, MT_EDIT_DIST, 3);
	//StartQuery(7, f2, MT_EXACT_MATCH, 0);
	//
	//	dfs(&(trie->root));
	//	EndQuery(7);
	////	dfs(&(trie->root));
	//	printf("done\n");

	//hashTest();
	MatchDocument(10, " ecooks     ");
	//	MatchDocument(11, "ok no fucker");
	//	MatchDocument(20, "fuck you oknofutcher");
	//	MatchDocument(30, "fuck mother you oknofucker father");
	DocID did;
	QueryID *qid;
	unsigned int numRes;
	GetNextAvailRes(&did, &numRes, &qid);

	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
//	GetNextAvailRes(&did, &numRes, &qid);
}
