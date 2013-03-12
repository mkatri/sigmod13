#define byte char

typedef struct {
	int* keys;
	void** pointers;byte currClusterSize;
	void* next;
} HashCluster;

typedef struct {
	HashCluster * *table;
} HashTable;

void hashTest();
HashTable* new_Hash_Table();
void insert(HashTable* ht, int key, void* ptr);
void* get(HashTable* ht, int key);
void delete_H(HashTable* ht, int key);
