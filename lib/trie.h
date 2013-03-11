/*
 * Trie.h
 *
 *  Created on: Mar 10, 2013
 *      Author: me2amet
 */

#ifndef TRIE_H_
#define TRIE_H_
#include "linked_list.h"
#define CHAR_SET_SIZE 26
#define BASE_CHAR 'a'

typedef struct TrieNode {
	struct TrieNode* next[CHAR_SET_SIZE];
	LinkedList_t* list;
	int count[3];
} TrieNode_t;
typedef struct Trie {
	TrieNode_t root;
} Trie_t;

DNode_t* TrieInsert(Trie_t * trie, char * str, int length, int type,
		void* queryData);
TrieNode_t* next(TrieNode_t *current, char c);
TrieNode_t * newTrieNode();

#endif /* TRIE_H_ */
