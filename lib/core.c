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

///////////////////////////////////////////////////////////////////////////////////////////////
Trie_t *trie;

/*QUERY DESCRIPTOR MAP GOES HERE*/
QueryDescriptor* qmap[1000000];
inline QueryDescriptor * getQueryDescriptor(int queryId) {
	return qmap[queryId];
}
inline void addQuery(int queryId, QueryDescriptor * qds) {
	qmap[queryId] = qds;
}
/*QUERY DESCRIPTOR MAP ENDS HERE*/

void init() {
	trie = newTrie();
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

	char queryWords[6][32];
	int wordSizes[6];
	int numOfWords = 0;
	int numOfSegments = match_dist + 1;

	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = newQueryDescriptor();
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;

	addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryWords, query_str, &numOfWords);
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
		char * word = (char *) malloc((wordLength + 1) * sizeof(char));
		for (i = 0; i < wordLength; i++) {
			word[i] = queryWords[in][i];
		}

		word[i] = '\0';

		// add the word in it's location in the query descriptor words array
		queryDescriptor->words[in] = word;

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
			for (j = 0; j < first; j++) {
				segment[j] = word[iq];
				iq++;
			}

			segment[j] = '\0';
			//load the segment data
			sd->queryId = query_id;
			sd->startIndex = iq - first;
			sd->wordIndex = in;

			//insert in trie
			queryDescriptor->segmentsData[top++] = TrieInsert(trie, segment,
					first, match_type, sd);

		}

		// loop on the word to get the segments
		for (i = 0; i < numOfSegments - k; i++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			for (j = 0; j < second; j++) {
				segment[j] = word[iq];
				iq++;
			}
			segment[j] = '\0';
			//load segments data
			sd->queryId = query_id;
			sd->startIndex = iq - second;
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

void split(int length[6], char output[6][32], const char* query_str, int * idx) {

	int iq = 0;

	*idx = 0;
	int idx2 = 0;

	// if the string is null
	//TODO i think we must fire error
	if (!query_str[iq])
		return;
	// assume there are spaces in the first
	while (query_str[iq] && query_str[iq] == ' ')
		iq++;

	// loop and get the words
	while (query_str[iq]) {

		if (query_str[iq] == ' ') {

			output[(*idx)][idx2] = '\0';
			length[(*idx)] = idx2;
			(*idx)++;
			idx2 = 0;

			while (query_str[iq] && query_str[iq] == ' ')
				iq++;

		}

		output[(*idx)][idx2++] = query_str[iq];
		iq++;
	}
	output[(*idx)][idx2] = '\0';
	length[(*idx)] = idx2;
	(*idx)++;
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

	while (doc_str[i]) {
		while (doc_str[i] == ' ')
			i++;

		e = i;
		while (doc_str[e] != ' ')
			e++;

		matchWord(&doc_str[i], e - i);

		i += e;
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	return EC_SUCCESS;
}

///////////////////////////////////////////
void core_test() {
	init();
	char output[32][32];

	char f[32] = "mother";
	char f2[32] = "oknofucker";

	StartQuery(0, f, 1, 0);
	StartQuery(1, f2, 1, 0);
//	dfs(&(trie->root));
	EndQuery(0);
	puts("---------------------------");
	puts("---------------------------");
	puts("---------------------------");

//	dfs(&(trie->root));
	//	printo(f);
	//	int num = 0;
	//	getSegments(output, f, 11, 3, &num);
	//	puts(output[3]);
	//	printf("done %d", num);
}
