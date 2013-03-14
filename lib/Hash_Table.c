#include "Hash_Table.h"

size_t size = 19210;

HashTable* new_Hash_Table() {
	HashTable* ht = (HashTable*) malloc(sizeof(HashTable));
	ht->table = (void**) malloc(size * sizeof(void*));
	return ht;
}

void insert(HashTable* ht, int id, void* ptr) {
	ht->table[id] = ptr;
}

void* get(HashTable* ht, int id) {
	return ht->table[id];
}

void delete_H(HashTable* ht, int id) {
	ht->table[id] = 0;
}
