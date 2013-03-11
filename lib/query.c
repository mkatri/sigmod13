#include "query.h"

SegmentData * newSegmentdata() {
	SegmentData *ret = (SegmentData*) malloc(sizeof(SegmentData));
	ret->queryId = 0;
	ret->wordIndex = 0;
	ret->startIndex = 0;
	return ret;
}

QueryDescriptor * newQueryDescriptor() {
	QueryDescriptor * ret = (QueryDescriptor*) malloc(sizeof(QueryDescriptor));
	ret->matchDistance = 0;
	ret->matchType = 0;
	ret->matchedWords = 0;
	return ret;
}
