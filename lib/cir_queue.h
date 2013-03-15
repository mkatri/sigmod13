/*
 * cir_queue.h
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */

#ifndef CIR_QUEUE_H_
#define CIR_QUEUE_H_
#include <pthread.h>

typedef struct {
	int front;
	int tail;
	int size;
	int cap;
	void **array;
	pthread_mutex_t lock;
	pthread_cond_t not_full;
	pthread_cond_t not_empty;
	pthread_cond_t full;
} CircularQueue;

inline void cir_queue_init(CircularQueue *q, void **array, int cap);
inline void cir_queue_insert(CircularQueue *q, void *i);
inline void *cir_queue_remove(CircularQueue *q);
inline void waitTillFull(CircularQueue *q);

#endif /* CIR_QUEUE_H_ */
