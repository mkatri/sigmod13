#include "Edit_Distance.h"
#include "word.h"

int nodeCnt = 0, init_ = 1000000;

int poolCnt = 0;

ED_Trie_Node* getNode(Edit_Distance *ed) {
	ED_Trie_Node *node = (*(ed->node_pool))++;

	node->next = (*(ed->next_pool)) += 27;

	nodeCnt++;
	if (nodeCnt == init_) {
		ed->node_pool = (ED_Trie_Node**) malloc(sizeof(ED_Trie_Node*));
		*(ed->node_pool) = (ED_Trie_Node*) malloc(init_ * sizeof(ED_Trie_Node));

		ed->next_pool = (ED_Trie_Node**) malloc(sizeof(ED_Trie_Node*));
		*(ed->next_pool) = (ED_Trie_Node*) malloc(
				init_ * 27 * sizeof(ED_Trie_Node));
		nodeCnt = 0;
	}
	return node;
}

Edit_Distance* new_Edit_Distance() {
	Edit_Distance* ed = (Edit_Distance*) malloc(sizeof(Edit_Distance));

	ed->trie = (ED_Trie*) malloc(sizeof(ED_Trie));
	ed->node_pool = (ED_Trie_Node**) malloc(sizeof(ED_Trie_Node*));
	*(ed->node_pool) = (ED_Trie_Node*) malloc(init_ * sizeof(ED_Trie_Node));

	ed->next_pool = (ED_Trie_Node**) malloc(sizeof(ED_Trie_Node*));
	*(ed->next_pool) = (ED_Trie_Node*) malloc(
			init_ * 27 * sizeof(ED_Trie_Node));

	ed->trie->root = getNode(ed);
	return ed;
}

int get_editDistance(int tid, char*a, int na, char*b, int nb, Edit_Distance* ed) {
	ED_Trie_Node* node = ed->trie->root;
	int i, c;
	if (currNode[tid] != 0)
		node = currNode[tid];
	else {
		for (i = 0; i < na; ++i) {
			c = a[i] - 'a';
			if (!node->next[c])
				return -1;
			node = node->next[c];
		}
	}

	if (!node->next[26])
		return -1;
	node = node->next[26];

	for (i = 0; i < nb; ++i) {
		c = b[i] - 'a';
		if (!node->next[c])
			return -1;
		node = node->next[c];
	}

	if (!node)
		return -1;

	return node->dist;
}

inline ED_Trie_Node* add_editDistance(char*a, int ia, char*b, int nb, int* T,
		Edit_Distance *ed, ED_Trie_Node* node) {
	int i, c = a[ia - 1] - 'a';

	if (!node->next[c])
		node->next[c] = getNode(ed);

	ED_Trie_Node* res = node = node->next[c];

	if (!node->next[26]) {
		node->next[26] = getNode(ed);
	}

	node = node->next[26];
	node->dist = T[0];

	for (i = 0; i < nb; ++i) {
		c = b[i] - 'a';
		if (!node->next[c]) {
			break;
		}
		node = node->next[c];
	}

	for (; i < nb; ++i) {
		c = b[i] - 'a';
		node->next[c] = getNode(ed);
		node = node->next[c];
		node->dist = T[i + 1];
	}

	return res;
}
