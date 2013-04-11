/*
 * Trie.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */
#ifndef TRIE_H_
#define TRIE_H_
#include "linked_list.h"
#define CHAR_SET_SIZE 35
#define BASE_CHAR 'a'
#include "query.h"
//#include <pthread.h>

typedef struct TrieNode2 {
	struct TrieNode2* next[CHAR_SET_SIZE];
	char terminal;
	int docId;
	long long word_time;
	LinkedList_t* list;
} TrieNode_t2;

typedef struct Trie2 {
	TrieNode_t2 root;
	TrieNode_t2 *pool;
	unsigned long pool_size;
	unsigned long pool_space;
	TrieNode_t2 returned;
	unsigned char spinLock;
} Trie_t2;

typedef struct TrieNode3 {
	struct TrieNode3* next[26 + 1];
	LinkedList_t list;
	long long node_time;
} TrieNode3;
typedef struct Trie3 {
	TrieNode3 root;
	TrieNode3 *pool;
	unsigned long pool_size;
	unsigned long pool_space;
	TrieNode3 returned;
	unsigned char spinLock;
} Trie3;

Trie3 * newTrie3();
TrieNode3 * newTrieNode3();

DNode_t* InsertTrie3(Trie3 *trie, char * str, int length, SegmentData* segData);

Trie_t2 * newTrie2();

TrieNode_t2 * newTrieNode2();
void TrieDelete2(Trie_t2* trie);
long long TriewordExist(Trie_t2* trie, char * str, int length, int docId,
		LinkedList_t** list);

#endif /* TRIE_H_ */
