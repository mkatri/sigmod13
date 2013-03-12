#define CORE_DEBUG
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <core.h>
#include "query.h"
#include "trie.h"
#include "document.h"

///////////////////////////////////////////////////////////////////////////////////////////////
Trie_t *trie;
LinkedList_t *docList;

/*QUERY DESCRIPTOR MAP GOES HERE*/
QueryDescriptor* qmap[1000000];
inline QueryDescriptor * getQueryDescriptor(int queryId) {
	return qmap[queryId];
}
inline void addQuery(int queryId, QueryDescriptor * qds) {
	qmap[queryId] = qds;
}
/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

void init() {
	trie = newTrie();
	docList = newLinkedList();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode InitializeIndex() {
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void printWords(char out[6][32], int num) {
	int i = 0;
	for (i = 0; i < num; i++)
		puts(out[i]);
}

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {
#ifdef CORE_DEBUG
	printf("query: %d --> %s\n", query_id, query_str);
#endif

	//TODO DNode_t ** segmentsData ;
	int in = 0, i = 0, j = 0, wordLength = 0, k, first, second, iq = 0;

	int wordSizes[6];
	int numOfWords = 0;
	int numOfSegments = match_dist + 1;

	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = newQueryDescriptor();
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;

	addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryDescriptor, query_str, &numOfWords);
	queryDescriptor->numWords = numOfWords;
//	printf("%d\n",wordSizes[1]);
//	return 0;

	char segment[32];
	/*initialize the DNode array here*/
	int top = 0;
	queryDescriptor->segmentsData = (DNode_t**) malloc(
			numOfSegments * numOfWords * sizeof(DNode_t *));
	for (top = 0; top < numOfSegments * numOfWords; top++)
		queryDescriptor->segmentsData[top] = 0;
	top = 0;
	for (in = 0; in < numOfWords; in++) {
		//get the word length
		iq = 0;
		wordLength = wordSizes[in];
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
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					first, match_type, sd);

		}

		// loop on the word to get the segments
		for (i = 0; i < numOfSegments - k; i++) {
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
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					second, match_type, sd);
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
			length[(*idx)] = idx1;
			(*idx)++;
			words[*idx] = &output[idx2];
			idx1 = 0;
			while (query_str[iq] && query_str[iq] == ' ')
				iq++;

		}

		output[idx2++] = query_str[iq];
		idx1++;
		iq++;
	}

	length[(*idx)] = idx1;
	(*idx)++;
	words[*idx] = &output[idx2];
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id) {

	puts("inside here");
	QueryDescriptor* queryDescriptor = getQueryDescriptor(query_id);

	int i, j;
	int in, iq, wordLength, numOfSegments = queryDescriptor->matchDistance + 1,
			k, first, second;
	char segment[32];
	int top = 0;
	for (in = 0; in < 5 && queryDescriptor->words[in]; in++) {
		//get the word length
		iq = 0;
		wordLength = strlen(queryDescriptor->words[in]);
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
			for (j = 0; j < first; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
			//Delete from the trie
			TrieDelete(trie, segment, first, queryDescriptor->matchType);
		}

		// loop on the word to get the segments
		for (i = 0; i < numOfSegments - k; i++) {
			for (j = 0; j < second; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top++]); //TODO ALSO DELETE SEGMENT DATA inside the node
			//Delete from the trie
			TrieDelete(trie, segment, first, queryDescriptor->matchType);
		}
	}
	freeQueryDescriptor(queryDescriptor);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	int i = 0, e = 0;
	int queryMatchCount = 0;

	while (doc_str[i]) {
		while (doc_str[i] == ' ')
			i++;

		e = i;
		while (doc_str[e] != ' ' && doc_str[e] != '\0')
			e++;

		matchWord(&doc_str[i], e - i, &queryMatchCount);

		i = e;
	}

	void *alloc = malloc(
			sizeof(DocumentDescriptor) + sizeof(QueryID) * queryMatchCount);
	DocumentDescriptor *doc_desc = alloc + sizeof(QueryID) * queryMatchCount;
	doc_desc->docId = doc_id;
	doc_desc->matches = alloc;
	doc_desc->numResults = queryMatchCount;
	int p = 0;
	for (i = 0; i < 1000000; i++) {
		if (qmap[i]) {
			if (qmap[i]->matchedWords == (1 << (qmap[i]->numWords)) - 1) {
				printf("doc %d matched query %d\n", doc_id, i);
				doc_desc->matches[p++] = i; //since qmap is a map, i is the QueryID
			}
			qmap[i]->matchedWords = 0;
		}
	}

	append(docList, doc_desc);
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	DNode_t *node = docList->head.next;
	DocumentDescriptor* doc_desc = (DocumentDescriptor *) (node->data);
	*p_query_ids = doc_desc->matches;
	*p_doc_id = doc_desc->docId;
	*p_num_res = doc_desc->numResults;
	//TODO if numRes is zero, free doc_desc
	delete(node);
	return EC_SUCCESS;
}

///////////////////////////////////////////
void core_test() {
	init();
	char output[32][32];

	char f[32] = "mother";
	char f2[32] = "oknofucker";

	StartQuery(5, f, MT_EDIT_DIST, 3);
	StartQuery(7, f2, MT_EDIT_DIST, 2);
	//dfs(&(trie->root));
	MatchDocument(10, "yomother fucker");
	MatchDocument(20, "fuck you oknofutcher");
	MatchDocument(30, "fuck mother you oknofucker father");
	DocID did;
	QueryID *qid;
	unsigned int numRes;
	GetNextAvailRes(&did, &numRes, &qid);
	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
	GetNextAvailRes(&did, &numRes, &qid);
	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
	GetNextAvailRes(&did, &numRes, &qid);
	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
//	EndQuery(0);
//	puts("---------------------------");
//	puts("---------------------------");
//	puts("---------------------------");

//	dfs(&(trie->root));
	//	printo(f);
	//	int num = 0;
	//	getSegments(output, f, 11, 3, &num);
	//	puts(output[3]);
	//	printf("done %d", num);
}
