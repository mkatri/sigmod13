#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/core.h"
#include "trie.h"
TrieNode_t * newTrieNode() {
	TrieNode_t* ret = (TrieNode_t*) (malloc(sizeof(TrieNode_t)));
	memset(ret->next, 0, sizeof(ret->next));
	memset(ret->list1, 0, sizeof(ret->list1));
	memset(ret->list2, 0, sizeof(ret->list2));
//		ret->list = 0;
	memset(ret->count, 0, sizeof(ret->count));
	return ret;
}
Trie_t * newTrie() {
	Trie_t* t = (Trie_t *) malloc(sizeof(Trie_t));
	memset(t->root.count, 0, sizeof(t->root.count));
//	t->root.list = 0;
	memset(t->root.list1, 0, sizeof(t->root.list1));
	memset(t->root.list2, 0, sizeof(t->root.list2));
	memset(t->root.next, 0, sizeof(t->root.next));
	return t;
}

inline TrieNode_t* next_node(TrieNode_t *current, char c) {
	if (current == 0)
		return 0;
	if (c >= 'a' && c <= 'z')
		return current->next[c - BASE_CHAR];
}

inline TrieNode_t2* next_node2(TrieNode_t2 *current, char c) {
	return current->next[c - BASE_CHAR];
}
DNode_t* TrieInsert(Trie_t * trie, char * str, int length, int type,
		void* queryData, int wordLength) {
#ifdef CORE_DEBUG
	puts(str);
#endif
	TrieNode_t* current = &(trie->root);
	current->count[type]++;
	int i;
	TrieNode_t* p = &(trie->root);
	for (i = 0; i < length; i++) {
		if (current->next[str[i] - BASE_CHAR] == 0)
			current->next[str[i] - BASE_CHAR] = newTrieNode();
		current = current->next[str[i] - BASE_CHAR];
		current->count[type]++;
		p = current;
	}
	if (type == MT_EDIT_DIST) {
		if (current->list1[wordLength] == 0)
			current->list1[wordLength] = newLinkedList();
		return append(current->list1[wordLength], queryData);
	} else {
		if (current->list2[wordLength] == 0)
			current->list2[wordLength] = newLinkedList();
		return append(current->list2[wordLength], queryData);
	}
}

//void deleteTrieNode(TrieNode_t* node) {
//#ifdef CORE_DEBUG
//	printf("DELETING NODE\n");
//#endif
//	if (node->list != 0)
//		free(node->list);
//	free(node);
//}

//NODE END QUERY BEFORE CALLING THIS FUNCTION MUST DELETE ALL LINKEDLIST NODES BELONGING TO SUCH QUERY
void TrieDelete(Trie_t* trie, char*str, int length, int type) {
	TrieNode_t* current = &(trie->root);
#ifdef CORE_DEBUG
	printf("----> deleting %d characters\n", length);
	puts(str);
#endif
	int i;
	for (i = 0; i < length; i++) {
		TrieNode_t *next = next_node(current, str[i]);
		next->count[type]--;
		if (next->count[0] + next->count[1] + next->count[2] == 0) {
			current->next[str[i] - BASE_CHAR] = 0;
		}
//		if (current->count[0] + current->count[1] + current->count[2] == 0
//				&& current != &(trie->root)) {
//			deleteTrieNode(current);
//		}
		current = next;
	}
//Alternative Implementation:Delete LinkedList node here (note:full traversal is required)
//	if (current->count[0] + current->count[1] + current->count[2] == 0
//			&& current != &(trie->root)) { //Note the check if current!=&(trie.root) is not really required unless we are kidding (LOL)
//		deleteTrieNode(current);
//	}
}

// NEW TRIE !
//--------------------------------------------------------------------------------------------------------------
TrieNode_t2 * newTrieNode2() {
	TrieNode_t2* ret = (TrieNode_t2*) (malloc(sizeof(TrieNode_t2)));
	memset(ret->next, 0, sizeof(ret->next));
	ret->terminal = 0;
	return ret;
}
char TriewordExist(Trie_t2* trie, char * str, int length, int docId,
		LinkedList_t **matchList) {
	TrieNode_t2 *cur = &(trie->root);
	int i;
	for (i = 0; i < length; i++)
		if (cur->next[str[i] - BASE_CHAR] != 0)
			cur = cur->next[str[i] - BASE_CHAR];
		else {
			for (; i < length; i++) {
				if (cur->next[str[i] - BASE_CHAR] == 0) {
					cur->next[str[i] - BASE_CHAR] = newTrieNode2();
					cur->next[str[i] - BASE_CHAR]->terminal |=
							(i == length - 1);
					cur->next[str[i] - BASE_CHAR]->docId = docId;
				}
				cur = cur->next[str[i] - BASE_CHAR];
			}
			cur->terminal = 1;
			cur->docId = docId;
			//TODO this is never freed, stupidly :D
			cur->matchList = newLinkedList();
			*matchList = cur->matchList;
			return 0;
		}
	if (cur->terminal) {
		if (cur->docId == docId) {
			*matchList = cur->matchList;
			return 1;
		}
		cur->docId = docId;
	} else {
		cur->terminal = 1;
		cur->docId = docId;
	}
	//TODO this is never freed, stupidly :D
	cur->matchList = newLinkedList();
	*matchList = cur->matchList;
	return 0;
}

void dfs(TrieNode_t * node) {
	int i;
#ifdef CORE_DEBUG
	printf("%d %d %d\n", node->count[0], node->count[1], node->count[2]);
#endif
	for (i = 0; i < CHAR_SET_SIZE; i++) {
		if (node->next[i] != 0) {
			putchar(i + 'a');
			dfs(node->next[i]);
		}
	}
}
//int main(int argc, char **argv) {
//	Trie_t *t = newTrie();
//	char str[50] = "hello";
//	char str2[20] = "motherfucker";
//	char str3[23] = "hellobitch";
//	TrieInsert(t, str, 5, 0, 0);
//	TrieInsert(t, str2, 12, 1, 0);
//	TrieInsert(t, str3, 10, 2, 0);
//	dfs(&(t->root));
//	puts("DONE");
//	TrieDelete(t, str3, 10, 2);
//	dfs(&(t->root));
//	return 0;
//}

