/*
 * word.h
 *
 *  Created on: Apr 10, 2013
 *      Author: mkatri
 */

#ifndef WORD_H_
#define WORD_H_
#include "trie.h"
#include <stdlib.h>
#include <stdio.h>
void matchWord(int did, int tid, char *w, int l, int *count, Trie3 * trie3,
		LinkedList_t* list, long long word_time, long long doc_word_num);
#endif /* WORD_H_ */
