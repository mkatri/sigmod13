/*
 * treap.c
 *
 *  Created on: 13 Mar 2013
 *      Author: kimo
 *      ref emaxx
 */

#include "treap.h"
#include <sys/time.h>
#define MAX_PRIORITY (1<<30);

int count = 0;
TNode_t* newTNode() {
	TNode_t * ret = (TNode_t *) malloc(sizeof(TNode_t));
	memset(ret, 0, sizeof(TNode_t));
	return ret;
}

Treap_t* newTreap() {
	Treap_t* ret = (Treap_t*) malloc(sizeof(Treap_t));
	ret->root = 0;
	return ret;
}


void splitTree(TNode_t* t, int key, TNode_t** l, TNode_t** r) {
	if (!t)
		(*l) = (*r) = 0;
	else if (key < (t->key))
		splitTree(t->l, key, l, &(t->l)), (*r) = t;
	else
		splitTree(t->r, key, &(t->r), r), (*l) = t;
}

void add(TNode_t** t, TNode_t * it) {
//iterative
	//	while (1) {
//			if (!(*t)) {
//				*t = it;
//				return;
//			}
//			if (it->prior > (*t)->prior) {
//				splitTree(*t, it->key, &(it->l), &(it->r)), (*t) = it;
//				return;
//			}
//			t = (it->key < (*t)->key) ? &((*t)->l) : &((*t)->r);
//		}

	//recursive
	if (!(*t))
		*t = it;
	else if (it->prior > (*t)->prior)
		split(*t, it->key, &(it->l), &(it->r)), (*t) = it;
	else
		insert(it->key < (*t)->key ? &((*t)->l) : &((*t)->r), it);
}

void merge(TNode_t** t, TNode_t * l, TNode_t* r) {
//iterative
//	while (1) {
//		if (!l || !r) {
//			(*t) = l ? l : r;
//			return;
//		} else if (l->prior > r->prior) {
//			(*t) = l;
//			t = &(l->r);
//			l = l->r;
//		} else {
//			(*t) = r;
//			t = &(l->r);
//			r = r->l;
//		}
//
//	}

	//recursive
	if (!l || !r)
		(*t) = l ? l : r;
	else if (l->prior > r->prior)
		merge(&(l->r), l->r, r), (*t) = l;
	else
		merge(&(r->l), l, r->l), (*t) = r;

}

void erase(TNode_t ** t, int key) {
//iterative
//	while (*t) {
//		if ((*t)->key == key) {
//			merge(t, (*t)->l, (*t)->r);
//			return;
//		}
//		t = (key < (*t)->key) ? &(*t)->l : &(*t)->r;
//	}

//recursive
	if(*t)
	{
	if ((*t)->key == key)
		merge(t, (*t)->l, (*t)->r);
	else
		erase(key < (*t)->key ? &((*t)->l) : &((*t)->r), key);
	}
}

void print(TNode_t* root, int i) {
	//printf("%lld\n",root);
	if (root) {
//		cout<<"key =  "<<root->key<<"pir= "<<root->prior<<endl;
		printf("key= %d  pir= %d  depth = %d \n", root->key, root->prior, i);
		print(root->l, i + 1);
		print(root->r, i + 1);
	}
}

void TreapInsert(Treap_t* treap, int key) {

	TNode_t * n1 = newTNode();
	n1->key = key;
	n1->prior = rand() % MAX_PRIORITY;

	add(&(treap->root), n1);
}

void TreapErase(Treap_t* treap, int key) {

	erase(&(treap->root), key);
}

int GetClockTimeInMilliSec() {
	struct timeval t2;
	gettimeofday(&t2, 0);
	return t2.tv_sec * 1000 + t2.tv_usec / 1000;
}
void PrintTime(int milli_sec) {
	int v = milli_sec;
	int hours = v / (1000 * 60 * 60);
	v %= (1000 * 60 * 60);
	int minutes = v / (1000 * 60);
	v %= (1000 * 60);
	int seconds = v / 1000;
	v %= 1000;
	int milli_seconds = v;
	int first = 1;
	printf("%d[", milli_sec);
	if (hours) {
		if (!first)
			printf(":");
		printf("%dh", hours);
		first = 0;
	}
	if (minutes) {
		if (!first)
			printf(":");
		printf("%dm", minutes);
		first = 0;
	}
	if (seconds) {
		if (!first)
			printf(":");
		printf("%ds", seconds);
		first = 0;
	}
	if (milli_seconds) {
		if (!first)
			printf(":");
		printf("%dms", milli_seconds);
		first = 0;
	}
	printf("]");
}

int testTreap() {

	int v = GetClockTimeInMilliSec();
	Treap_t * treap = newTreap();
	int i = 0;
	printf("..insert..\n");
	for (i = 0; i < 100000000; i++) {
		//	printf("%d\n",i);
		TreapInsert(treap, i);

	}
i=0;
printf("..delete..\n");
	for(i=0;i<100000000;i++)
	{

		TreapErase(treap,i);

	}
	v = GetClockTimeInMilliSec() - v;
	printf("Time=");
	PrintTime(v);
	printf("\n");

//	TreapInsert(treap, 0);
//	TreapInsert(treap, 1);
//	TreapInsert(treap, 2);
//	TreapInsert(treap, 3);
//	TreapInsert(treap, 6);
//	TreapInsert(treap, 5);
//	printf("done\n");
//	TreapErase(treap, 5);
//	TreapErase(treap, 0);
//	TreapErase(treap, 1);
//	TreapErase(treap, 2);
//	TreapErase(treap, 3);
//	TreapErase(treap, 5);
//	printf("done\n");
//	print(treap->root, 0);


	return 0;
}

