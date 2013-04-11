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

struct QueryDescriptor;

typedef struct {
	int queryId;
	struct QueryDescriptor* parentQuery;
	unsigned char wordIndex;
	char* startIndex;
} SegmentData;

typedef struct QueryDescriptor {
	char *words[6];
	char segmentSizes[6][6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
//	__attribute__((align(64))) struct {
//		char matchedWords;
//		char docId;
//	} thSpec[NUM_THREADS];
	char matchedWords[NUM_THREADS];
	int docId[NUM_THREADS];
//	DNode_t ** segmentsData;
	SegmentData segments[5];
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
} QueryDescriptor;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

