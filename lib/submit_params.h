/*
 * threading.h
 *
 *  Created on: Mar 14, 2013
 *      Author: mkatri
 */

#ifndef THREADING_H_
#define THREADING_H_


#define HOME

#ifdef HOME
#define QDESC_MAP_SIZE 2000
#define RES_POOL_INITSIZE 10000
#define NUM_THREADS 1
#define HT_SIZE  5000011

#else
#define QDESC_MAP_SIZE 20000
#define RES_POOL_INITSIZE 10000000
#define NUM_THREADS 24
#define HT_SIZE 1000000007

#endif

#endif /* THREADING_H_ */
