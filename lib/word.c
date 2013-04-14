#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "submit_params.h"
#include "linked_list.h"
#include "trie.h"
#include "query.h"
#include "word.h"
#include "atomic.h"
//char matched[QDESC_MAP_SIZE ][6];
TrieNode_t2 * dtrieQueue[INIT_QUEUE_SIZE ];
TrieNode3 * qtrieQueue[INIT_QUEUE_SIZE ];

pthread_mutex_t debug_lock;
pthread_mutex_t queue_lock;
pthread_cond_t queue_not_empty;
pthread_cond_t queue_is_empty;

int size;
int ptr;
int p;
int waiting_threads;
char reset[NUM_THREADS];

int gdid;
int *gcount;
int gtask_size;
LinkedList_t *gresults;

unsigned char gcount_spinLock;

void *thread(void *n);

pthread_t bfs_threads[NUM_THREADS];

extern long long overhead;
extern long long total;

inline int bsf(int bitmask) {
	int first = 0;
	int isZero = -1;
	asm(
			"bsf %1, %0\n\t"
			"cmove %2, %0"
			:"=r"(first)
			:"g"(bitmask), "rm"(isZero)
			:);

	return first;
}

inline long bsfl(long bitmask) {
	long first = 0;
	long isZero = -1;
	asm(
			"bsf %1, %0\n\t"
			"cmove %2, %0"
			:"=r"(first)
			:"g"(bitmask), "rm"(isZero)
			:);

	return first;
}

void initialize_matchTrie() {
	pthread_mutex_init(&debug_lock, NULL );
	pthread_mutex_init(&queue_lock, NULL );
	pthread_cond_init(&queue_is_empty, NULL );
	pthread_cond_init(&queue_not_empty, NULL );
	int i;
	for (i = 0; i < NUM_THREADS; i++)
		pthread_create(&bfs_threads[i], NULL, thread, (void *) (uintptr_t) i);
}

void threaded_matchTrie(int did, int *count, int task_size, TrieNode_t2 *dTrie,
		TrieNode3 *qTrie, LinkedList_t *results) {
//TODO make sure the threads really started :D
	pthread_mutex_lock(&queue_lock);
	dtrieQueue[0] = dTrie;
	qtrieQueue[0] = qTrie;
	size = 1;
	ptr = 0;
	p = 1;
	waiting_threads = 0;
	gdid = did;
	gcount = count;
	gtask_size = task_size;
	gresults = results;
	memset(reset, 1, sizeof(char) * NUM_THREADS);
	pthread_cond_signal(&queue_not_empty);
	while (waiting_threads < NUM_THREADS)
		pthread_cond_wait(&queue_is_empty, &queue_lock);
	pthread_mutex_unlock(&queue_lock);
}

void *thread(void *n) {
	uintptr_t tid = (uintptr_t) n;
	TrieNode_t2 *dTrie;
	TrieNode3 *qTrie;

	while (1) {
		pthread_mutex_lock(&queue_lock);
		while (size == 0) {
			waiting_threads++;
			pthread_cond_signal(&queue_is_empty);
			pthread_cond_wait(&queue_not_empty, &queue_lock);
			if (!reset[tid]) {
				waiting_threads--;
			} else {
				reset[tid] = 0;
			}
		}
		dTrie = dtrieQueue[ptr];
		qTrie = qtrieQueue[ptr];
		ptr++;
		size--;

		int band = (qTrie->qmask & dTrie->dmask), dmask = dTrie->dmask, ind;

		TrieNode3* star = qTrie->next[26];

		while ((ind = bsf(dmask)) > -1) {
			dmask ^= (1 << ind);
			if (star) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = star;
				size++;
			}
			if (band & (1 << ind)) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = qTrie->next[ind];
				size++;
			}
		}
		pthread_cond_signal(&queue_not_empty);
		pthread_mutex_unlock(&queue_lock);

		if (dTrie->terminal == 1 && qTrie && !isEmpty(&(qTrie->list))
				&& (qTrie->done != gdid
						|| (qTrie->done_bitmask
								!= (((uint64_t) 1L << gtask_size) - 1)))) {
			while (xchg(&qTrie->spinLock, 1))
				;
			if (qTrie->done != gdid) {
				qTrie->done_bitmask = 0;
				qTrie->done = gdid;
			}
			long notDoneFingerPrint = dTrie->fingerPrint & ~qTrie->done_bitmask;
			qTrie->done_bitmask |= dTrie->fingerPrint;
			qTrie->spinLock = 0;

			if (notDoneFingerPrint != 0) {
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData;
				QueryDescriptor * queryData;
				int ok = 0, tmp = 0;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;
					tmp++;

					while (xchg(&queryData->spinLock, 1))
						;
					if (queryData->thSpec.docId != gdid) {
						queryData->thSpec.docId = gdid;
						memset(queryData->thSpec.docsMatchedWord, 0,
								5 * sizeof(long));
					}

					long localNotDoneFingerPrint =
							notDoneFingerPrint
									& ~queryData->thSpec.docsMatchedWord[segData->wordIndex];

					if (localNotDoneFingerPrint != 0) {
						ok = 1;
						int w;
						long oldQueryStatus, newQueryStatus;
						oldQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							oldQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						//TODO check this bitch? :D
						queryData->thSpec.docsMatchedWord[segData->wordIndex] |=
								notDoneFingerPrint;

						newQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							newQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						queryData->spinLock = 0; //TODO replace :D

						if (oldQueryStatus == 0 && newQueryStatus != 0) {
							sync_append(gresults,
									(void *) (uintptr_t) queryData);
						}

						long d;
						while (xchg(&gcount_spinLock, 1))
							;
						while ((d = bsfl(localNotDoneFingerPrint)) > -1) {
							localNotDoneFingerPrint ^= (1L << d);
							if (((oldQueryStatus & (1L << d)) == 0)
									&& ((newQueryStatus & (1L << d)) != 0))
								gcount[d]++;
						}
						gcount_spinLock = 0;

					} else {
						queryData->spinLock = 0;
					}
					cur = cur->next;
				}
				if (!ok)
					overhead += tmp;
				total += tmp;
			}

		}

	}
	return 0;
}

void matchTrie(int did, int *count, int task_size, TrieNode_t2 *dTrie,
		TrieNode3 *qTrie, LinkedList_t *results) {
	dtrieQueue[0] = dTrie;
	qtrieQueue[0] = qTrie;
	int size = 1;
	int ptr = 0;
	int p = 1;
	while (size != 0) {
		qTrie = qtrieQueue[ptr];
		dTrie = dtrieQueue[ptr];
		ptr++;
		size--;

		int band = (qTrie->qmask & dTrie->dmask), dmask = dTrie->dmask, ind;

		TrieNode3* star = qTrie->next[26];

		while ((ind = bsf(dmask)) > -1) {
			dmask ^= (1 << ind);
			if (star) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = star;
				size++;
			}
			if (band & (1 << ind)) {
				dtrieQueue[p] = dTrie->next[ind];
				qtrieQueue[p++] = qTrie->next[ind];
				size++;
			}
		}

		if (dTrie->terminal == 1 && qTrie && !isEmpty(&(qTrie->list))
				&& (qTrie->done != did
						|| (qTrie->done_bitmask
								!= (((uint64_t) 1L << task_size) - 1)))) {

			if (qTrie->done != did) {
				qTrie->done_bitmask = 0;
				qTrie->done = did;
			}

			long notDoneFingerPrint = dTrie->fingerPrint & ~qTrie->done_bitmask;

			if (notDoneFingerPrint != 0) {
				DNode_t *cur = qTrie->list.head.next;
				SegmentData * segData = (SegmentData *) (cur->data);
				QueryDescriptor * queryData = segData->parentQuery;
				int ok = 0, tmp = 0;
				while (cur != &(qTrie->list.tail)) {
					segData = (SegmentData *) (cur->data);
					queryData = segData->parentQuery;
					tmp++;
					if (queryData->thSpec.docId != did) {
						queryData->thSpec.docId = did;
						memset(queryData->thSpec.docsMatchedWord, 0,
								5 * sizeof(long));
					}

					long localNotDoneFingerPrint =
							notDoneFingerPrint
									& ~queryData->thSpec.docsMatchedWord[segData->wordIndex];

					if (localNotDoneFingerPrint != 0) {
						ok = 1;
						int w;
						long oldQueryStatus, newQueryStatus;
						oldQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							oldQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						//TODO check this bitch? :D
						queryData->thSpec.docsMatchedWord[segData->wordIndex] |=
								notDoneFingerPrint;

						newQueryStatus = queryData->thSpec.docsMatchedWord[0];
						for (w = 1; w < queryData->numWords; w++)
							newQueryStatus &=
									queryData->thSpec.docsMatchedWord[w];

						if (oldQueryStatus == 0 && newQueryStatus != 0) {
							append(results, (void *) (uintptr_t) queryData);
						}

						long d;
						while ((d = bsfl(localNotDoneFingerPrint)) > -1) {
							localNotDoneFingerPrint ^= (1L << d);
							if (((oldQueryStatus & (1L << d)) == 0)
									&& ((newQueryStatus & (1L << d)) != 0))
								count[d]++;
						}
					}
					cur = cur->next;
				}
				if (!ok)
					overhead += tmp;
				total += tmp;

			}

			qTrie->done_bitmask |= dTrie->fingerPrint;
		}
	}
}
