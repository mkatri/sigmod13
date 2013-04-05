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

typedef struct TrieNode {
	struct TrieNode* next[CHAR_SET_SIZE];
	LinkedList_t* list1[32];
	LinkedList_t* list2[32];
	LinkedList_t* edit_dist_list[32];
	int max_dist[32];
	struct TrieNode* edit_dist_Trie;
	int count[3];
	int counter;
} TrieNode_t;
typedef struct Trie {
	TrieNode_t root;
} Trie_t;
typedef struct TrieNode2 {
	struct TrieNode2* next[CHAR_SET_SIZE];
	char terminal;
	int docId;
} TrieNode_t2;
typedef struct Trie2 {
	TrieNode_t2 root;
} Trie_t2;


DNode_t* TrieInsert(Trie_t * trie, char * str, char* word, int length, int type,
		SegmentData* queryData, int wordLength, int s, int e);
void TrieDelete(Trie_t* trie, char*str, int length, int type);

inline TrieNode_t* next_node(TrieNode_t *current, char c);
TrieNode_t * newTrieNode();
Trie_t * newTrie();
Trie_t2 * newTrie2();

TrieNode_t2 * newTrieNode2();
void TrieDelete2(Trie_t2* trie);
char TriewordExist(Trie_t2* trie, char * str, int length, int docId);

#endif /* TRIE_H_ */
