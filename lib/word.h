#include "Edit_Distance.h"
#include "submit_params.h"

int ok;
int editDistance(int tid, char* a, int na, char* b, int nb, int dist);
ED_Trie_Node* currNode[NUM_THREADS];
