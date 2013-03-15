/*
 * Query.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef QUERY_H_
#define QUERY_H_
#include "submit_params.h"
#include "linked_list.h"
#include <core.h>

typedef struct {
	int queryId;
	struct QueryDescriptor_S* parentQuery;
	/*left matching */
	unsigned char segmentIndex;
	/*end of left matching */
	unsigned char wordIndex;
	char* startIndex;
	DNode_t *trieNode;
} SegmentData;

typedef struct QueryDescriptor_S {
	char *words[6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	char matchedWords[NUM_THREADS];
	/*left matching */
	char part_matchedWords[NUM_THREADS];
	unsigned char next_seg[NUM_THREADS][5];
	unsigned char cur_dist[NUM_THREADS][5];
	char *docWord[NUM_THREADS];
	struct QueryDescriptor_S *prev[NUM_THREADS];
	struct QueryDescriptor_S *next[NUM_THREADS];
	/*end of left matching */
	DocID docId[NUM_THREADS];
	SegmentData *segmentsData[5][4];
	char numWords;
	char matchType;
	char matchDistance;
	QueryID queryId;
} QueryDescriptor;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

