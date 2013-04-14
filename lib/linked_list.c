#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "submit_params.h"
#include "atomic.h"

LinkedList_t default_pool;
long appendCount;
long deleteCount;

void initLinkedListDefaultPool() {
	default_pool.head.next = 0;
	default_pool.tail.next = (DNode_t*) malloc(
			sizeof(DNode_t) * INIT_LLPOOL_SIZE);
	default_pool.head.prev = default_pool.tail.next;
	default_pool.tail.prev = default_pool.tail.next + INIT_LLPOOL_SIZE;
	default_pool.spinLock = 0;
}

void initLinkedListPool(LinkedList_t *pool, int size) {
	pool->head.next = 0;
	pool->tail.next = (DNode_t*) malloc(sizeof(DNode_t) * size);
	pool->head.prev = pool->tail.next;
	pool->tail.prev = pool->tail.next + size;
	pool->spinLock = 0;
}

LinkedList_t* newLinkedList() {
	LinkedList_t* ret = (LinkedList_t*) memalign(64, sizeof(LinkedList_t));
//	LinkedList_t* ret;
//	posix_memalign((void **)&ret, 64, sizeof(LinkedList_t));
	memset(ret, 0, sizeof(LinkedList_t));
	ret->head.next = &(ret->tail), ret->tail.prev = &(ret->head);
	return ret;
}

inline void lock_pool(LinkedList_t *pool) {
	while (xchg(&(pool->spinLock), 1))
		;
}

inline void unlock_pool(LinkedList_t *pool) {
	pool->spinLock = 0;
}

inline DNode_t* alloc_node(LinkedList_t *pool) {
	DNode_t* node;
	if (pool->head.next) {
		node = pool->head.next;
		pool->head.next = node->next;
	} else {
		if (pool->tail.next == pool->tail.prev) {
			//XXX expand size exponentially?
			unsigned long newPoolSize = (pool->tail.prev - pool->head.prev);
			pool->tail.next = (DNode_t*) malloc(sizeof(DNode_t) * newPoolSize);
			pool->head.prev = pool->tail.next;
			pool->tail.prev = pool->tail.next + newPoolSize;
		}

		node = (DNode_t*) pool->tail.next;
		pool->tail.next++;
	}
	return node;
}

inline void dealloc_node(DNode_t *node, LinkedList_t *pool) {
	node->next = pool->head.next;
	pool->head.next = node;
}

DNode_t* append(LinkedList_t* list, void * data) {
	lock_pool(&default_pool);
	DNode_t* node = alloc_node(&default_pool);
	unlock_pool(&default_pool);
	node->prev = list->tail.prev, node->next = &(list->tail);
	node->next->prev = node, node->prev->next = node;
	node->data = data;
	return node;
}

DNode_t* append_with_pool(LinkedList_t *list, void * data, LinkedList_t *pool) {
	DNode_t* node = alloc_node(pool);
	node->prev = list->tail.prev, node->next = &(list->tail);
	node->next->prev = node, node->prev->next = node;
	node->data = data;
	return node;
}

DNode_t* sync_append(LinkedList_t* list, void * data) {
	lock_pool(&default_pool);
	DNode_t* node = alloc_node(&default_pool);
	unlock_pool(&default_pool);
	while (xchg(&(list->spinLock), 1))
		;
	node->prev = list->tail.prev, node->next = &(list->tail);
	node->next->prev = node, node->prev->next = node;
	list->spinLock = 0;
	node->data = data;
	return node;
}

DNode_t* delete_node(DNode_t * node) {
	node->next->prev = node->prev;
	node->prev->next = node->next;
	DNode_t* nxt = node->next;
	lock_pool(&default_pool);
	dealloc_node(node, &default_pool);
	unlock_pool(&default_pool);
	return nxt;
}

DNode_t* delete_node_with_pool(DNode_t * node, LinkedList_t* pool) {
	node->next->prev = node->prev;
	node->prev->next = node->next;
	DNode_t* nxt = node->next;
	dealloc_node(node, pool);
	return nxt;
}

char isEmpty(LinkedList_t * list) {
	return list == 0 || list->head.next == &list->tail;
}

