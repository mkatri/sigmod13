#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

TrieNode_t * newTrieNode() {
	TrieNode_t* ret = (TrieNode_t*) (malloc(sizeof(TrieNode_t)));
	memset(ret->next, 0, sizeof(ret->next));
	ret->list = 0;
	memset(ret->count, 0, sizeof(ret->count));
	ret->isTerminal = 0;
	return ret;
}
Trie_t * newTrie() {
	Trie_t* t = (Trie_t *) malloc(sizeof(Trie_t));
	memset(t->root.count, 0, sizeof(t->root.count));
	t->root.list = 0;
	memset(t->root.next, 0, sizeof(t->root.next));
	t->root.isTerminal = 0;
	return t;
}

inline TrieNode_t* next_node(TrieNode_t *current, char c) {
	return current->next[c - BASE_CHAR];
}

inline TrieNode_t2* next_node2(TrieNode_t2 *current, char c) {
	return current->next[c - BASE_CHAR];
}
DNode_t* TrieInsert(Trie_t * trie, char * str, int length, int type,
		int lstart, int lend, int rstart, int rend, void* queryData) {
	TrieNode_t* current = &(trie->root);
	current->count[type]++;
	int i;
	for (i = 0; i < length; i++) {
		if (current->next[str[i] - BASE_CHAR] == 0)
			current->next[str[i] - BASE_CHAR] = newTrieNode();
		current = current->next[str[i] - BASE_CHAR];
		current->count[type]++;
	}

	if (current->list == 0)
		current->list = newLinkedList();

	if (current->leftTrie == NULL)
		current->leftTrie = malloc(sizeof(TrieNode_t));
	if (current->rightTrie == NULL)
		current->rightTrie = malloc(sizeof(TrieNode_t));

	current->isTerminal = 1;

	DNode_t* listNode = append(current->list, queryData);

	return listNode;
}
void deleteTrieNode(TrieNode_t* node) {
#ifdef CORE_DEBUG
	printf("DELETING NODE\n");
#endif
	if (node->list != 0)
		free(node->list);
	free(node);
}

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
		if (current->count[0] + current->count[1] + current->count[2] == 0
		/*&& current != &(trie->root)*/) {
			current->isTerminal = 0;
			//			deleteTrieNode(current);
		}
		current = next;
	}
	//	Alternative Implementation:Delete LinkedList node here (note:full traversal is required)
	if (current->count[0] + current->count[1] + current->count[2] == 0
	/*&& current != &(trie->root)*/) { //Note the check if current!=&(trie.root) is not really required unless we are kidding (LOL)
		current->isTerminal = 0;
		//		deleteTrieNode(current);
	}
}

// NEW TRIE !
//--------------------------------------------------------------------------------------------------------------
TrieNode_t2 * newTrieNode2() {
	TrieNode_t2* ret = (TrieNode_t2*) (malloc(sizeof(TrieNode_t2)));
	memset(ret->next, 0, sizeof(ret->next));
	ret->c = 0;
	ret->terminal = 0;
	return ret;
}

Trie_t2 * newTrie2() {
	Trie_t2* t = (Trie_t2 *) malloc(sizeof(Trie_t2));
	memset(t->root.next, 0, sizeof(t->root.next));
	t->root.c = 0;
	t->root.terminal = 0;
	return t;
}
void TrieInsert2(Trie_t2* trie, char * str, int length, int docId) {
	TrieNode_t2 *cur = &(trie->root);
	int i;
	for (i = 0; i < length; i++) {
		if (cur->next[str[i] - BASE_CHAR] == 0) {
			cur->next[str[i] - BASE_CHAR] = newTrieNode2();
			cur->next[str[i] - BASE_CHAR]->c = str[i] - BASE_CHAR;
			cur->next[str[i] - BASE_CHAR]->terminal |= (i == length - 1);
			cur->next[str[i] - BASE_CHAR]->docId = docId;
		}
		cur = cur->next[str[i] - BASE_CHAR];
	}
}
char TriewordExist(Trie_t2* trie, char * str, int length, int docId) {
	TrieNode_t2 *cur = &(trie->root);
	int i;
	for (i = 0; i < length; i++)
		if (cur->next[str[i] - BASE_CHAR] != 0)
			cur = cur->next[str[i] - BASE_CHAR];
		else
			return 0;
	if (cur->terminal) {
		if (cur->docId == docId)
			return 1;
		cur->docId = docId;
	} else {
		cur->terminal = 1;
		cur->docId = docId;
	}
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

