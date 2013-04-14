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
//#define PROFILER
#define DOC_PER_TASK 64
#define QDESC_MAP_SIZE (int)5E4
#define TRIE2_INIT_SIZE 1E4 /* this is the actual maximum in small AND big test */
#define TRIE3_INIT_SIZE 1E5
#define INIT_LLPOOL_SIZE (int)1E5
#define INIT_RESPOOL_SIZE (int)1E4
#define INIT_DOCPOOL_SIZE (int)1E2
#define INIT_QUEUE_SIZE (int)5E5
//#define CONC_TRIE3

#else
#define DOC_PER_TASK 48
#define QDESC_MAP_SIZE (int)3E6 /* TODO this WILL bite us in the final test */
#define TRIE2_INIT_SIZE 1E6 /* 4 is the actual maximum in small AND big test */
#define TRIE3_INIT_SIZE 1E7 /* 1 billion items */
#define INIT_LLPOOL_SIZE (int)1E7
#define INIT_RESPOOL_SIZE (int)1E3
#define INIT_DOCPOOL_SIZE (int)1E4
#define INIT_QUEUE_SIZE (int)5E5

//#define CONC_TRIE3

#endif

#endif /* THREADING_H_ */
