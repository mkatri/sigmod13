#ifndef SH
#define SH

#include <stdlib.h>
#include <stdio.h>

typedef struct {
	struct ED_Trie_Node** next;
	int dist;
} ED_Trie_Node;

typedef struct {
	ED_Trie_Node* root;
} ED_Trie;

typedef struct {
	ED_Trie* trie;
	ED_Trie_Node** node_pool;
	ED_Trie_Node** next_pool;
} Edit_Distance;

Edit_Distance* new_Edit_Distance();

int get_editDistance(int tid, char*a, int na, char*b, int nb,
		Edit_Distance* sh);

ED_Trie_Node* add_editDistance(char*a, int ia, char*b, int nb, int* T,
		Edit_Distance *ed, ED_Trie_Node* node);

#endif
