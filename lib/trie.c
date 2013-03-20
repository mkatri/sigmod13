#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

TrieNode_t * newTrieNode() {
	TrieNode_t* ret = (TrieNode_t*) (malloc(sizeof(TrieNode_t)));
	memset(ret->next, 0, sizeof(ret->next));
	int i, j;

	memset(ret->count, 0, sizeof(ret->count));
	ret->isTerminal = 0;
	return ret;
}
Trie_t * newTrie() {
	Trie_t* t = (Trie_t *) malloc(sizeof(Trie_t));
	memset(t->root.count, 0, sizeof(t->root.count));
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

DNode_t* insertParts(TrieNode_t** n, int type, int dist, int l, int r,
		char* str, void* queryData, int* newPart) {
	TrieNode_t* node = n[0];
	int i;
	*newPart = 0;
	for (i = l; i < r; i++) {
		if (node->next[str[i] - BASE_CHAR] == 0) {
			if (i == r - 1)
				*newPart = 1;
			node->next[str[i] - BASE_CHAR] = newTrieNode();
		}
		node = node->next[str[i] - BASE_CHAR];
	}

	n[0] = node;

	if (node->SegmentDataList == 0)
		node->SegmentDataList = newLinkedList();

	DNode_t* itr = node->SegmentDataList->head.next;
	while (itr != &(node->SegmentDataList->tail)) {
		SegmentData* segdata = itr->data;
		if (type < segdata->parentQuery->matchType)
			break;
		if (type == segdata->parentQuery->matchType && dist
				< segdata ->parentQuery->matchDistance)
			break;
	}

	itr = itr->prev;
	DNode_t* Listnode = (DNode_t *) (malloc(sizeof(DNode_t)));
	Listnode->data = queryData;

	Listnode->next = itr->next;
	Listnode->prev = itr;
	itr->next = Listnode;
	itr->next->prev = Listnode;

	return Listnode;
}

DNode_t** TrieInsert(Trie_t * trie, char * str, int length, int type, int dist,
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

	current->isTerminal = 1;

	if (!current->PartsTrie)
		current->PartsTrie = newTrieNode();

	TrieNode_t* node = current->PartsTrie;
	int newPart = 0;
	DNode_t** ret = malloc(2 * sizeof(DNode_t*));

	//===============================================
	partsNode* data = malloc(sizeof(partsNode));
	data->isRight = 0;
	data->len = lend - lstart;
	data->queryData = queryData;
	data->startChar = str[0];
	//insert leftPart
	ret[0] = insertParts(&node, type, dist, lstart, lend, str, data, &newPart);
	if (newPart)
		append(node->partsNodesList, node);
	node = current->PartsTrie;
	//===============================================
	data = malloc(sizeof(partsNode));
	data->isRight = 1;
	data->len = rend - rstart;
	data->queryData = queryData;
	data->startChar = str[rstart];
	//insert rightPart
	ret[1] = insertParts(&node, type, dist, rstart, rend, str, data, &newPart);
	if (newPart)
		append(node->partsNodesList, node);
	//===============================================
	return ret;
}
void deleteTrieNode(TrieNode_t* node) {
#ifdef CORE_DEBUG
	printf("DELETING NODE\n");
#endif
	if (node->SegmentDataList != 0)
		free(node->SegmentDataList);
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

