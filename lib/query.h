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
	char *words[6];
	char segmentSizes[6][6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	char matchedWords[NUM_THREADS];
	int docId[NUM_THREADS];
	DNode_t ** segmentsData;
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
} QueryDescriptor;

typedef struct {
	int queryId;
	QueryDescriptor* parentQuery;
	unsigned char wordIndex;
	char* startIndex;
} SegmentData;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

