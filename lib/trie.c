#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <core.h>
#include "trie.h"
TrieNode_t * newTrieNode() {
	TrieNode_t* ret = (TrieNode_t*) (malloc(sizeof(TrieNode_t)));
	memset(ret->next, 0, sizeof(ret->next));
	int tmp = sizeof(ret->list1);
	memset(ret->list1, 0, tmp);
	memset(ret->list2, 0, tmp);
	memset(ret->edit_dist_list, 0, tmp);
	memset(ret->count, 0, sizeof(ret->count));
	memset(ret->max_dist, 0, sizeof(ret->max_dist));
	ret->edit_dist_Trie = 0;
	ret->counter = 0;
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

Trie_t2 * newTrie2() {
	Trie_t2* t = (Trie_t2 *) malloc(sizeof(Trie_t2));
	memset(t, 0, sizeof(Trie_t2));
	return t;
}

TrieNode_t* next_node(TrieNode_t *current, char c) {
	if (current == 0)
		return 0;
	if (c >= 'a' && c <= 'z')
		return current->next[c - BASE_CHAR];
	return 0;
}

inline TrieNode_t2* next_node2(TrieNode_t2 *current, char c) {
	return current->next[c - BASE_CHAR];
}

DNode_t* insertParts(TrieNode_t** n, int type, int dist, int len, char* str,
		SegmentData* queryData, int s, int e) {

	TrieNode_t* node = n[0];
	n[0] = 0;
	int i;
	for (i = 0; i < len; i++) {
		if (node->next[str[i] - BASE_CHAR] == 0)
			node->next[str[i] - BASE_CHAR] = newTrieNode();
		node = node->next[str[i] - BASE_CHAR];
	}

	if (node->next[s] == 0)
		node->next[s] = newTrieNode();
	node = node->next[s];

	if (node->next[e] == 0)
		node->next[e] = newTrieNode();
	node = node->next[e];
	if (node->edit_dist_list[len] == 0)
		node->edit_dist_list[len] = newLinkedList();

	if (node->max_dist[len] < dist)
		node->max_dist[len] = dist;

	int empty = isEmpty(node->edit_dist_list[len]);

	DNode_t* ret = append(node->edit_dist_list[len], queryData);

	if (empty)
		n[0] = node;
	else
		ret->tmp = node->edit_dist_list[len]->head.next->tmp;

	return ret;
}

int cnt3 = 0, cnt4 = 0;

DNode_t* TrieInsert(Trie_t * trie, char * str, char* word, int length, int type,
		SegmentData* queryData, int wordLength, int s, int e) {
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

	current->counter++;
	if (type == MT_EDIT_DIST) {
		if (current->list1[wordLength] == 0)
			current->list1[wordLength] = newLinkedList();

		if (current->edit_dist_Trie == 0)
			current->edit_dist_Trie = newTrieNode();

		TrieNode_t* node[1];
		node[0] = current->edit_dist_Trie;

		DNode_t* ret = insertParts(node, type,
				queryData->parentQuery->matchDistance, wordLength, word,
				queryData, s, e);

		if (node[0])
			ret->tmp = append(current->list1[wordLength], node[0]);

		return ret;
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
	current->counter++;
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
char TriewordExist(Trie_t2* trie, char * str, int length, int docId) {
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
			return 0;
		}
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

