#include <stdlib.h>
#include <stdio.h>
#define byte char

typedef struct {
	void * *table;
} HashTable;

void hashTest();
HashTable* new_Hash_Table();
void insert(HashTable* ht, int key, void* ptr);
void* get(HashTable* ht, int key);
void delete_H(HashTable* ht, int key);
