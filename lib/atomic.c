/*
 * atomic.c
 *
 *  Created on: Apr 9, 2013
 *      Author: mkatri
 */

#include "atomic.h"
inline unsigned char cmpxchg(void *dest, uintptr_t oldVal, uintptr_t newVal) {
	unsigned char ret;
	asm __volatile("lock; cmpxchgq %2, %1\n\t" // dummy, semaphore (src, dest); AT/T syntax != Intel Syntax
			"sete %0\n"
			:"=r"(ret), "=m"(*dest)
			:"r"(newVal), "a"(oldVal)
			:"memory", "cc");

	return ret;
}

inline unsigned char xchg(unsigned char *dest, unsigned char newVal) {
	asm __volatile("lock; xchg %0, %1" // dummy, semaphore (src, dest); AT/T syntax != Intel Syntax
			:"=m"(*dest), "=r"(newVal)
			:"1"(newVal)
			:"memory");
	return newVal;
}
