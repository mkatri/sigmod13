#include <stdlib.h>
#include <stdio.h>
#include "Hash_Table.h"

typedef struct {
	HashTable* ht;ll* modpow;
} StringHashing;

int get_editDistance(char*a, int na, char*b, int nb, int* hash,
		StringHashing* sh);

void add_editDistance(char*a, int na, char*b, int nb, int dist, int hash,
		StringHashing* sh);
