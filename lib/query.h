/*
 * Query.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef QUERY_H_
#define QUERY_H_
#include "linked_list.h"
#include "../include/core.h"

typedef struct {
	char *words[6];
	char queryString[MAX_WORD_LENGTH * MAX_QUERY_WORDS + 1];
	char matchedWords;
	DNode_t ** segmentsDataL;
	DNode_t ** segmentsDataR;
	char numWords;
	int matchType;
	int matchDistance;
	int queryId;
	int docId;
} QueryDescriptor;

typedef struct {
	int queryId;
	QueryDescriptor* parentQuery;
	char wordIndex;
	char rightMatched;
	char leftMatched;
	char reminderDistance;
	int docId;
	char* startIndex;
} SegmentData;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

