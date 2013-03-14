/*
 * dyn_array.h
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */

#ifndef DYN_ARRAY_H_
#define DYN_ARRAY_H_
#include <core.h>

typedef struct {
	QueryID *array;
	int tail;
	int capacity;
} DynamicArray;

void dyn_array_init(DynamicArray *a, int init_capacity);
inline void dyn_array_insert(DynamicArray *a, QueryID i);

#endif /* DYN_ARRAY_H_ */
