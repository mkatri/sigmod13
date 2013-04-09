/*
 * atomic.h
 *
 *  Created on: Apr 9, 2013
 *      Author: mkatri
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_
#include <stdint.h>
inline unsigned char cmpxchg(uintptr_t *dest, uintptr_t oldVal,
		uintptr_t newVal);
inline unsigned char xchg(unsigned char *dest, unsigned char newVal);
#endif /* ATOMIC_H_ */
