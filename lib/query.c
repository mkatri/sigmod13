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
	memset(ret, 0, sizeof(QueryDescriptor));
	pthread_mutex_init(&ret->query_lock, NULL);
	return ret;
}

void freeQueryDescriptor(QueryDescriptor * qds) {
	pthread_mutex_destroy(&qds->query_lock);
	free(qds);

}
