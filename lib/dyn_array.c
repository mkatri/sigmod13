/*
 * dyn_array.c
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */
#include "dyn_array.h"

inline void doubleCapacity(DynamicArray *a) {
	a->capacity <<= 1;
	a->array = realloc(a->array, sizeof(QueryID) * a->capacity);
}

void dyn_array_init(DynamicArray *a, int init_capacity) {
	a->tail = 0;
	a->capacity = init_capacity;
	a->array = malloc(sizeof(QueryID) * init_capacity);
}

inline void dyn_array_insert(DynamicArray *a, QueryID i) {
	if (a->tail >= a->capacity)
		doubleCapacity(a);
	a->array[a->tail++] = i;
}
