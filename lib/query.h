/*
 * Query.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef QUERY_H_
#define QUERY_H_
#include <pthread.h>
#include "linked_list.h"
#include "../include/core.h"

typedef struct {
	char *words[6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	char matchedWords;
	pthread_mutex_t query_lock;
	DNode_t ** segmentsData;
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
} QueryDescriptor;

typedef struct {
	int queryId;
	QueryDescriptor* parentQuery;
	char wordIndex;
	char* startIndex;
} SegmentData;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

