/*
 * treap.h
 *
 *  Created on: 13 Mar 2013
 *      Author: kimo
 */

#ifndef TREAP_H_
#define TREAP_H_

#include "linked_list.h"
typedef struct TNode {
	int key, prior;
	struct TNode * l, * r;
	DNode_t * data;

}TNode_t;

typedef struct Treap{
	TNode_t *root;
}Treap_t;
typedef TNode_t * pitem;
TNode_t* newTNode();
Treap_t* newTreap();
void add(TNode_t** t, TNode_t * it);
void TreapInsert(Treap_t* treap, int key);
void TreapErase(Treap_t* treap, int key);
void erase(TNode_t ** t, int key) ;
int testTreap();



#endif /* TREAP_H_ */
