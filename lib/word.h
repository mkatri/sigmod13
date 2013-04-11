/*
 * word.h
 *
 *  Created on: Apr 10, 2013
 *      Author: mkatri
 */

#ifndef WORD_H_
#define WORD_H_
#include "trie.h"
#include <string.h>
void matchTrie(int did, int tid, int *count, TrieNode_t2 * dTrie,
		TrieNode3 * qTrie, LinkedList_t *results, LinkedList_t *pool);
void matchWord(int did, int tid, char *w, int l, int *count, Trie_t * trie,
		Trie3 * trie3, LinkedList_t *results, LinkedList_t *pool);
#endif /* WORD_H_ */
