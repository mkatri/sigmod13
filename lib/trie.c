#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core.h>
#include "linked_list.h"
#include "trie.h"
#include "atomic.h"

extern long long global_time;

Trie_t2 * newTrie2() {
	Trie_t2* t = (Trie_t2 *) malloc(sizeof(Trie_t2));
	memset(t, 0, sizeof(Trie_t2));
	t->pool_size = TRIE2_INIT_SIZE;
	t->pool = (TrieNode_t2 *) malloc(sizeof(TrieNode_t2) * t->pool_size);
	t->pool_space = t->pool_size;
	return t;
}

Trie3 * newTrie3() {
	Trie3* t = (Trie3 *) malloc(sizeof(Trie3));
	memset(t, 0, sizeof(Trie3));
	t->pool_size = TRIE3_INIT_SIZE;
	t->pool = (TrieNode3 *) malloc(sizeof(TrieNode3) * t->pool_size);
	t->pool_space = t->pool_size;
	return t;
}

void returnToPool(Trie3 *t, TrieNode3 *node) {
	while (xchg(&(t->spinLock), 1))
		;
	node->next[0] = t->returned.next[0];
	t->returned.next[0] = node;
	t->spinLock = 0;
}

TrieNode3 * newTrieNode3(Trie3 *t) {
	TrieNode3* ret;
	while (xchg(&(t->spinLock), 1))
		;
	if (t->returned.next[0]) {
		ret = t->returned.next[0];
		t->returned.next[0] = ret->next[0];
	} else {
		if (t->pool_space == 0) {
			t->pool_size *= 2;
			t->pool = (TrieNode3 *) malloc(sizeof(TrieNode3) * t->pool_size);
			t->pool_space = t->pool_size;
		}

		ret = (TrieNode3*) t->pool;
		t->pool++;
		t->pool_space--;
	}
	t->spinLock = 0;
	memset(ret, 0, sizeof(TrieNode3));
	ret->list.head.next = &(ret->list.tail), ret->list.tail.prev =
			&(ret->list.head);
	return ret;
}

DNode_t* InsertTrie3(Trie3 * trie, char * str, int length, SegmentData* segData) {
	TrieNode3* current = &(trie->root);
	int i;
	for (i = 0; i < length; i++) {
		if (current->next[str[i] - BASE_CHAR] == 0) {
			TrieNode3 *newNode = newTrieNode3(trie);
			if (!cmpxchg(&(current->next[str[i] - BASE_CHAR]), (uintptr_t) 0,
					(uintptr_t) newNode))
				returnToPool(trie, newNode);
		}
		current = current->next[str[i] - BASE_CHAR];
	}
	DNode_t* ret = sync_append(&(current->list), segData);
	return ret;
}

inline TrieNode_t2* next_node2(TrieNode_t2 *current, char c) {
	return current->next[c - BASE_CHAR];
}

// NEW TRIE !
//--------------------------------------------------------------------------------------------------------------
TrieNode_t2 * newTrieNode2(Trie_t2 *t) {
	TrieNode_t2 *ret;
	//XXX we never return those nodes
	if (t->pool_space == 0) {
		//XXX does not seem we need to double this one
		t->pool_size *= 2;
		t->pool = (TrieNode_t2 *) malloc(sizeof(TrieNode_t2) * t->pool_size);
		t->pool_space = t->pool_size;
	}

	ret = (TrieNode_t2*) t->pool;
	t->pool++;
	t->pool_space--;
	memset(ret, 0, sizeof(TrieNode_t2));
	return ret;
}

long long TriewordExist(Trie_t2* trie, char * str, int length, int docId,
		LinkedList_t** list) {
	TrieNode_t2 *cur = &(trie->root);
	int i;
	for (i = 0; i < length; i++)
		if (cur->next[str[i] - BASE_CHAR] != 0)
			cur = cur->next[str[i] - BASE_CHAR];
		else {
			for (; i < length; i++) {
				cur->next[str[i] - BASE_CHAR] = newTrieNode2(trie);
				cur->next[str[i] - BASE_CHAR]->terminal |= (i == length - 1);
				cur->next[str[i] - BASE_CHAR]->docId = docId;

				cur = cur->next[str[i] - BASE_CHAR];
				cur->terminal = 0;
			}
		}

	if (cur->terminal) {
		long long ret = cur->word_time;
		if (cur->docId == docId)
			ret *= -1;
		cur->docId = docId;
		cur->word_time = global_time;
		*list = cur->list;
		return ret;
	} else {
		cur->list = newLinkedList();
		*list = cur->list;
		cur->terminal = 1;
		cur->docId = docId;
		cur->word_time = global_time;
	}
	return 0;
}
