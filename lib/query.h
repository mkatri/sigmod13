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

struct compact {
	int docId;
	long docsMatchedWord[5];
};

typedef struct QueryDescriptor {
	char *words[6];
	char segmentSizes[6][6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	struct compact thSpec;
//	int docId[NUM_THREADS];
//	DNode_t ** segmentsData;
	SegmentData segments[5];
//	char matchedWords[NUM_THREADS];
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
	unsigned char spinLock;
} QueryDescriptor;
//TODO padd to 64 bytes when we dynamically allocate

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

