#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include "submit_params.h"
#include "atomic.h"

extern inline unsigned char cmpxchg(uintptr_t *dest, uintptr_t oldVal,
		uintptr_t newVal);
extern inline unsigned char xchg(unsigned char *dest, unsigned char newVal);

LinkedList_t default_pool;
long appendCount;
long deleteCount;

extern int cntz;

void initLinkedListDefaultPool() {
	default_pool.head.next = 0;
	default_pool.tail.next = (DNode_t*) malloc(
			sizeof(DNode_t) * INIT_LLPOOL_SIZE);
	default_pool.head.prev = default_pool.tail.next;
	default_pool.tail.prev = default_pool.tail.next + INIT_LLPOOL_SIZE;
	default_pool.spinLock = 0;
}

LinkedList_t* newLinkedList() {
	LinkedList_t* ret = (LinkedList_t*) malloc(sizeof(LinkedList_t));
	memset(ret, 0, sizeof(LinkedList_t));
	ret->head.next = &(ret->tail), ret->tail.prev = &(ret->head);
	return ret;
}

inline DNode_t* alloc_node(LinkedList_t *pool) {
	DNode_t* node;
	while (xchg(&(pool->spinLock), 1))
		;
	if (pool->head.next) {
		node = pool->head.next;
		pool->head.next = node->next;
	} else {
		if (pool->tail.next == pool->tail.prev) {
			//XXX expand size exponentially?
			unsigned long newPoolSize = (pool->tail.prev - pool->head.prev)
					* 1.5;
			pool->tail.next = (DNode_t*) malloc(sizeof(DNode_t) * newPoolSize);
			pool->head.prev = pool->tail.next;
			pool->tail.prev = pool->tail.next + newPoolSize;
		}

		node = (DNode_t*) pool->tail.next;
		pool->tail.next++;
	}
	pool->spinLock = 0;
	return node;
}

inline void dealloc_node(DNode_t *node, LinkedList_t *pool) {
	while (xchg(&(pool->spinLock), 1))
		;
	node->next = pool->head.next;
	pool->head.next = node;
	pool->spinLock = 0;
}

DNode_t* append(LinkedList_t* list, void * data) {
	DNode_t* node = alloc_node(&default_pool);
	node->prev = list->tail.prev, node->next = &(list->tail);
	node->next->prev = node, node->prev->next = node;
	node->data = data;
	return node;
}

/*DNode_t* append_from_pool(LinkedList_t *list, void * data, LinkedList_t *pool) {
 DNode_t* node = alloc_node(pool);
 node->prev = list->tail.prev, node->next = &(list->tail);
 node->next->prev = node, node->prev->next = node;
 node->data = data;
 return node;
 }*/

DNode_t* sync_append(LinkedList_t* list, void * data) {
	DNode_t* node = alloc_node(&default_pool);
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
	dealloc_node(node, &default_pool);
	return nxt;
}

char isEmpty(LinkedList_t * list) {
	return list == 0 || list->head.next == &list->tail;
}

