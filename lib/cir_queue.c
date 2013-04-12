/*
 * cir_queue.c
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */
#include "cir_queue.h"

void cir_queue_init(CircularQueue *q, void **array, int cap) {
	q->array = array;
	q->cap = cap;
	pthread_cond_init(&q->not_full, NULL );
	pthread_cond_init(&q->not_empty, NULL );
	pthread_mutex_init(&q->lock, NULL );
	q->front = 0;
	q->tail = 0;
	q->size = 0;
}

void cir_queue_lock(CircularQueue *q) {
	pthread_mutex_lock(&q->lock);
}

void cir_queue_unlock(CircularQueue *q) {
	pthread_mutex_unlock(&q->lock);
}

char cir_queue_nowait_insert(CircularQueue *q, void *i) {
	if (q->size >= q->cap)
		return 0;
	q->array[q->tail] = i;
	q->tail = (q->tail + 1) % q->cap;
	q->size++;
	if (q->size == q->cap)
		pthread_cond_signal(&q->full);
	pthread_cond_signal(&q->not_empty);
	return 1;
}

void *cir_queue_nowait_remove(CircularQueue *q) {
	if (q->size <= 0)
		return 0;
	void *ret = q->array[q->front];
	q->front = (q->front + 1) % q->cap;
	q->size--;
	pthread_cond_signal(&q->not_full);
	return ret;
}

void cir_queue_insert(CircularQueue *q, void *i) {
	pthread_mutex_lock(&q->lock);
	while (q->size >= q->cap)
		pthread_cond_wait(&q->not_full, &q->lock);
	q->array[q->tail] = i;
	q->tail = (q->tail + 1) % q->cap;
	q->size++;
	if (q->size == q->cap)
		pthread_cond_signal(&q->full);
	pthread_cond_signal(&q->not_empty);
	pthread_mutex_unlock(&q->lock);
}

void *cir_queue_remove(CircularQueue *q) {
	pthread_mutex_lock(&q->lock);
	while (q->size <= 0)
		pthread_cond_wait(&q->not_empty, &q->lock);
	void *ret = q->array[q->front];
	q->front = (q->front + 1) % q->cap;
	q->size--;
	pthread_cond_signal(&q->not_full);
	pthread_mutex_unlock(&q->lock);
	return ret;
}

void waitTillFull(CircularQueue *q) {
	pthread_mutex_lock(&q->lock);
	while (q->size != q->cap)
		pthread_cond_wait(&q->full, &q->lock);
	pthread_mutex_unlock(&q->lock);
}
