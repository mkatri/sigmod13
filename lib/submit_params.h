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

#else

#define THREAD_ENABLE
#define RES_POOL_INITSIZE 100000
#define NUM_THREADS 24
#define HT_SIZE 100007
#define QDESC_MAP_SIZE 5000000

#endif

#endif /* THREADING_H_ */
