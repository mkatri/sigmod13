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
	union {
		struct {
			int queryId;
			struct QueryDescriptor* parentQuery;
			unsigned char wordIndex;
			char* startIndex;
		};
		char padding[64];
	};
} SegmentData __attribute__ ((aligned (64)));

struct compact {
	union {
		struct {
			int docId;
			char matchedWords;
		};
		char padding[64];
	};
}__attribute__ ((aligned (64)));

typedef struct QueryDescriptor {
	char *words[6];
	char segmentSizes[6][6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	struct compact thSpec[NUM_THREADS] __attribute__ ((aligned (64)));
//	int docId[NUM_THREADS];
//	DNode_t ** segmentsData;
	SegmentData segments[5] __attribute__ ((aligned (64)));
//	char matchedWords[NUM_THREADS];
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
} QueryDescriptor __attribute__ ((aligned (64)));
//TODO padd to 64 bytes when we dynamically allocate

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

