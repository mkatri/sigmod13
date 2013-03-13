#include "Hash_Table.h"

size_t size = 1000000007;
char clusterSize = 8;

int hash(int key) {
	return key % size;
//	return ((long long)key * 2654435761l) % size;
	//===================
//	long long k = key;
//	k = ((k >> 16) ^ k) * 0x45d9f3bl;
//	k = ((k >> 16) ^ k) * 0x45d9f3bl;
//	k = ((k >> 16) ^ k);
//	return k % size;
}

HashTable* new_Hash_Table() {
	HashTable* ht = (HashTable*) malloc(sizeof(HashTable));
	ht->table = (HashCluster**) malloc(size * sizeof(HashCluster*));
	return ht;
}

HashCluster* getCluster() {
	HashCluster* clust = (HashCluster*) malloc(sizeof(HashCluster));

	clust->keys = (int*) malloc(clusterSize * sizeof(int));

	clust->pointers = malloc(clusterSize * sizeof(void*));

	int* k = clust->keys;
	k[0] = k[1] = k[2] = k[3] = -1;
	clust->currClusterSize = 0;
	clust->next = 0;

	return clust;
}

/* TODO */
void freeCluster(HashCluster* clust) {
	free(clust->keys);
	free(clust->pointers);
	free(clust);
}

void insert(HashTable* ht, int id, void* ptr) {
	int h = hash(id);

	HashCluster* clust = ht->table[h];

	if (clust == NULL)
		ht->table[h] = clust = getCluster();

	while (clust->currClusterSize == clusterSize && clust->next != NULL)
		clust = clust->next;

	if (clust->currClusterSize == clusterSize) {
		clust->next = getCluster();
		clust = clust->next;
	}

	int i = 0;
	while (clust->keys[i] != -1)
		i++;

	clust->keys[i] = id;
	clust->pointers[i] = ptr;
	clust->currClusterSize++;

}

void search(HashCluster** res, HashCluster** before, int key, int* index) {
	HashCluster * resClust = *res;
	HashCluster * beforeClust = *before;

	int i;
	while (resClust != NULL) {

		for (i = 0; i < clusterSize; ++i) {
			if (resClust->keys[i] == key) {
				*index = i;
				return;
			}
		}

		beforeClust = resClust;
		resClust = resClust->next;
	}

	*res = resClust;
	*before = beforeClust;
}

void* get(HashTable* ht, int id) {
	int h = hash(id), *i = (int*) malloc(sizeof(int));

	HashCluster* res = ht->table[h];

	HashCluster* prev = ht->table[h];
//	printf("====== %d %d %d %d\n", res->pointers[0], res->pointers[1],
//			res->pointers[2], res->pointers[3]);
	search(&res, &prev, id, i);
	int ii = *i;
	free(i);
//	printf("====== %d %d %d %d\n", res->keys[0], res->keys[1], res->keys[2],
//			res->keys[3]);
//	printf("====== %d\n",ii);
	if (res != NULL)
		return res->pointers[ii];

	return 0;
}

void delete_H(HashTable* ht, int id) {
	int h = hash(id), *i = (int*) malloc(sizeof(int));

	HashCluster* res = ht->table[h];
	HashCluster* beforeRes = res;
	search(&res, &beforeRes, id, i);
	if (res != NULL) {

		res->keys[*i] = -1;
		res->currClusterSize--;
//		printf("====== %d %d %d %d\n", res->keys[0], res->keys[1], res->keys[2],
//				res->keys[3]);
		if (res->currClusterSize == 0) {
			if (ht->table[h] == res) {
				ht->table[h] = res->next;
			} else {
				beforeRes->next = res->next;
			}
			freeCluster(res);
		}
	}
	free(i);
}

void hashTest() {
	HashTable* ht = new_Hash_Table();
	int i = 0;
	printf("======INSERTION======\n");
	for (i = 0; i < 10; ++i) {
		printf("%d\n", 0);
		insert(ht, 0, i + 5);
	}
	printf("=====GET=======\n");
	for (i = 0; i < 10; ++i) {
		printf("%d\n", 0);
		printf("pointer returned: %d\n", get(ht, 0));
	}
	printf("======DELETION======\n");
	for (i = 0; i < 10; ++i) {
		printf("%d\n", 0);
		delete_H(ht, 0);
	}
	printf("=====GET=======\n");
	for (i = 0; i < 10; ++i) {
		printf("%d\n", 0);
		printf("pointer returned: %d\n", get(ht, 0));
	}
}
