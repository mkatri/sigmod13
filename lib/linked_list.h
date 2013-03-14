/*
 * linkedList.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

typedef struct DNode {
	struct DNode *prev, *next;
	void * data;
} DNode_t;
typedef struct LinkedList {
	DNode_t head, tail;
} LinkedList_t;
/*creates a new empty linkedlist */
LinkedList_t* newLinkedList();
/**
 * inserts new element at the tail of list, with data
 * node it doesn't copy the data.
 */
DNode_t* append(LinkedList_t* list, void * data);
/**
 * deletes node from the linkedlist it belongs, and frees memory used by node
 * note:this doesn't free the memory used by content
 * returns the node->next (for deletion while iteration).
 */
DNode_t* delete(DNode_t * node);
/**
 * returns true if the linkedlist list is empty
 */
char isEmpty(LinkedList_t * list);

#endif /* LINKEDLIST_H_ */
