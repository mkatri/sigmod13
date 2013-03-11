#include "query.h"

SegmentData * newSegmentdata() {
	SegmentData *ret = (SegmentData*) malloc(sizeof(SegmentData));
	ret->queryId = 0;
	ret->parentQuery = 0;
	ret->wordIndex = 0;
	ret->startIndex = 0;
	return ret;
}

QueryDescriptor * newQueryDescriptor() {
	QueryDescriptor * ret = (QueryDescriptor*) malloc(sizeof(QueryDescriptor));
	ret->matchDistance = 0;
	ret->matchType = 0;
	ret->matchedWords = 0;
	memset(ret->words, 0, sizeof(char*));
	return ret;
}

void freeQueryDescriptor(QueryDescriptor * qds) {
	free(qds->segmentsData);
	int i;
	for (i = 0; i < 5 && qds->words[i]; i++)
		free(qds->words[i]);
	free(qds->words);
	free(qds);
}
