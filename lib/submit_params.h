/*
 * threading.h
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */

#ifndef THREADING_H_
#define THREADING_H_


//#define HOME

#ifdef HOME

#define RES_POOL_INITSIZE 10000
#define NUM_THREADS 2
#define HT_SIZE  5000011

#else

#define RES_POOL_INITSIZE 1000000
#define NUM_THREADS 4
#define HT_SIZE 100000007

#endif

#endif /* THREADING_H_ */
