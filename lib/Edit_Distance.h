#ifndef SH
#define SH

#include <stdlib.h>
#include <stdio.h>

typedef struct {
	struct ED_Trie_Node* next[27];
	int* dist;
} ED_Trie_Node;

typedef struct {
	ED_Trie_Node* root;
} ED_Trie;

typedef struct {
	ED_Trie* trie;
} Edit_Distance;

Edit_Distance* new_Edit_Distance();

int get_editDistance(char*a, int na, char*b, int nb, Edit_Distance* sh);

void add_editDistance(char*a, int na, char*b, int nb, int dist, int** T,
		Edit_Distance* sh);

#endif