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
//#define PROFILER
#define THREAD_ENABLE
#define RES_POOL_INITSIZE 10000
#define NUM_THREADS 2
#define HT_SIZE 5000011
#define QDESC_MAP_SIZE 50000
#define TRIE3_INIT_SIZE 1000000
#else

#define THREAD_ENABLE
#define RES_POOL_INITSIZE 10000000
#define NUM_THREADS 16
#define HT_SIZE 100000007
#define QDESC_MAP_SIZE 5000000
#define TRIE3_INIT_SIZE 1E9 /* 1 billion items */
#endif

#endif /* THREADING_H_ */
