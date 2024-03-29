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
	union {
		struct {
			struct TrieNode2* next[CHAR_SET_SIZE];
			//	struct TrieNode2* list[CHAR_SET_SIZE];
			//	char pos[CHAR_SET_SIZE];
			//	int at;
			int dmask;
			char terminal;
			int docId;
		};
		char padding[320];
	};
} TrieNode_t2;

typedef struct Trie2 {
	union {
		struct {
			TrieNode_t2 root;
			TrieNode_t2 *pool;
			unsigned long pool_size;
			unsigned long pool_space;
			TrieNode_t2 returned;
			unsigned char spinLock;
		};
		char padding[704];
	};
} Trie_t2 __attribute__ ((aligned (64)));

typedef struct TrieNode3 {
	struct TrieNode3* next[26 + 1];
	LinkedList_t list;
	//TODO first thing to sacrifice :D
	int done[NUM_THREADS_DOC];
	int qmask;
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

DNode_t* TrieInsert(Trie_t * trie, char * str, char* word, int length, int type,
		SegmentData* queryData, int wordLength, int s, int e);
void TrieDelete(Trie_t* trie, char*str, int length, int type);

TrieNode_t* next_node(TrieNode_t *current, char c);
TrieNode_t * newTrieNode();
Trie_t * newTrie();
Trie_t2 * newTrie2();

TrieNode_t2 * newTrieNode2();
void TrieDelete2(Trie_t2* trie);
void TrieDocInsert(Trie_t2* trie, char *str, int length, int docId);
char TriewordExist(Trie_t2* trie, char * str, int length, int docId);

#endif /* TRIE_H_ */
