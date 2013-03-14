#include "Edit_Distance.h"
#include "word.h"

ED_Trie_Node* getNode() {
	ED_Trie_Node*node = (ED_Trie_Node*) malloc(sizeof(ED_Trie_Node));
	node->dist = (int*) malloc(sizeof(int));
	memset(node->next, 0, sizeof(node->next));
	node->dist[0] = -1;
	return node;
}

Edit_Distance* new_Edit_Distance() {
	Edit_Distance* ed = (Edit_Distance*) malloc(sizeof(Edit_Distance));
	ed->trie = (ED_Trie*) malloc(sizeof(ED_Trie));
	ed->trie->root = getNode();

	return ed;
}
int cnt = 0;
int get_editDistance(char*a, int na, char*b, int nb, Edit_Distance* ed) {
	int i, c;
	ED_Trie_Node* node = ed->trie->root;
	for (i = 0; i < na; ++i) {
		c = a[i] - 'a';
		if (!node->next[c])
			return -1;
		node = node->next[c];
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

	return node->dist[0];
}

void add_editDistance(char*a, int na, char*b, int nb, int dist, int** T,
		Edit_Distance* ed) {
	int i, c;

	ED_Trie_Node* node = ed->trie->root;

	for (i = 0; i < na; ++i) {
		c = a[i] - 'a';
		if (!node->next[c])
			break;
		node = node->next[c];
	}

	for (; i < na; ++i) {
		c = a[i] - 'a';
		node->next[c] = getNode();
		node = node->next[c];
		cnt++;
	}

	//================
	if (!node->next[26]) {
		node->next[26] = getNode();
		cnt++;
	}

	node = node->next[26];
	//================

	for (i = 0; i < nb; ++i) {
		c = b[i] - 'a';

		if (!node->next[c]) {
			break;
		}
		node = node->next[c];
	}
	for (; i < nb; ++i) {
		c = b[i] - 'a';
		node->next[c] = getNode();
		node = node->next[c];
		cnt++;
	}

	node->dist[0] = dist;
}
