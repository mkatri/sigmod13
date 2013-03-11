/*
 * Query.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef QUERY_H_
#define QUERY_H_
#include "linked_list.h"

typedef struct {
	char*words[5];
	char matchedWords;
	DNode_t ** segmentsData;
	int matchType;
	int matchDistance;
} QueryDescriptor;

typedef struct {
	int queryId;
	QueryDescriptor* parentQuery;
	char wordIndex;
	int startIndex;
} SegmentData;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();
void freeQueryDescriptor(QueryDescriptor * qds);

#endif /* QUERY_H_ */

