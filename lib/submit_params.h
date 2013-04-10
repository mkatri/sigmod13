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
#define NUM_THREADS 2
#define QDESC_MAP_SIZE (int)5E4
#define TRIE2_INIT_SIZE 1E5 /* this is the actual maximum in small AND big test */
#define TRIE3_INIT_SIZE 1E6
#define INIT_LLPOOL_SIZE (int)1E7
#define INIT_DOCPOOL_SIZE (int)1E4
#define CONC_TRIE3

#else
#define THREAD_ENABLE
#define NUM_THREADS 22
#define QDESC_MAP_SIZE (int)4E6 /* TODO this WILL bite us in the final test */
#define TRIE2_INIT_SIZE 1E5 /* 4 is the actual maximum in small AND big test */
#define TRIE3_INIT_SIZE 1E7 /* 1 billion items */
#define INIT_LLPOOL_SIZE (int)1E7
#define INIT_DOCPOOL_SIZE (int)1E5
//#define CONC_TRIE3

#endif

#endif /* THREADING_H_ */
