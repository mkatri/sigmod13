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

void cir_queue_init(CircularQueue *q, void **array, int cap);
void cir_queue_insert(CircularQueue *q, void *i);
void *cir_queue_remove(CircularQueue *q);
void waitTillFull(CircularQueue *q);

void cir_queue_lock(CircularQueue *q);
void cir_queue_unlock(CircularQueue *q);
char cir_queue_nowait_insert(CircularQueue *q, void *i);
void *cir_queue_nowait_remove(CircularQueue *q);

#endif /* CIR_QUEUE_H_ */
