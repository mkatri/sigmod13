/*
 * Query.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef QUERY_H_
#define QUERY_H_
#include "linked_list.h"

struct QuerySegmentData {

};
typedef struct {
	char*words[5];
	char matchedWords;
	DNode_t ** segmentsData;
	int matchType;
	int matchDistance;
} QueryDescriptor;
typedef struct {
	int queryId;
	char wordIndex;
	int startIndex;
} SegmentData;

SegmentData * newSegmentdata();
QueryDescriptor * newQueryDescriptor();

#endif /* QUERY_H_ */





