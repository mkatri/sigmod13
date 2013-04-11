#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <core.h>
#include <unistd.h>
#include "query.h"
#include "trie.h"
#include "document.h"
#include "cir_queue.h"
#include "submit_params.h"
#include "linked_list.h"
#include "word.h"
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////// DOC THREADING STRUCTS //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
long long global_time = 1;
#define CIR_QUEUE_SIZE 12 * NUM_THREADS_QUERY

pthread_t matcher_threads[NUM_THREADS_DOC];
pthread_t candidate_gen_threads[NUM_THREADS_QUERY];
char documents[CIR_QUEUE_SIZE][MAX_DOC_LENGTH];
CircularQueue cirq_free_docs;
CircularQueue cirq_busy_docs;
CircularQueue cirq_busy_queries;
CircularQueue cirq_free_segments;
CircularQueue cirq_busy_segments;
char *free_docs[CIR_QUEUE_SIZE];
DocumentDescriptor *busy_docs[CIR_QUEUE_SIZE];
QueryDescriptor *busy_queries[CIR_QUEUE_SIZE];
char *free_segments[CIR_QUEUE_SIZE];
QueryDescriptor *busy_segments[CIR_QUEUE_SIZE];
pthread_mutex_t docList_lock;
pthread_cond_t docList_avail;
pthread_mutex_t trie_lock;

pthread_mutex_t big_debug_lock;
//DynamicArray matches[NUM_THREADS];
int cmpfunc(const QueryID * a, const QueryID * b);
//void generate_candidates(char * str, int len, int dist, SegmentData* segData);
void *generate_candidates(void *n);
///////////////////////////////////////////////////////////////////////////////////////////////
Trie_t2 * dtrie[NUM_THREADS_DOC];
DocumentDescriptor docList;
LinkedList_t *queries;
unsigned long docCount;
int cnttt = 0;
Trie3 * eltire;
char lamda = 'a' + 26;
int cntz = 0;
/*QUERY DESCRIPTOR MAP GOES HERE*/
QueryDescriptor qmap[QDESC_MAP_SIZE ];
DNode_t qnodes[QDESC_MAP_SIZE ];

DNode_t *lazy_nodes[QDESC_MAP_SIZE ];
LinkedList_t* lazy_list;

LinkedList_t * edit_list[QDESC_MAP_SIZE ];

inline void addQuery(int queryId, QueryDescriptor * qds) {
	DNode_t* node = &qnodes[queryId];
	node->data = qds;
	node->prev = queries->tail.prev, node->next = &(queries->tail);
	node->next->prev = node, node->prev->next = node;
}

inline void removeQuery(int queryId, QueryDescriptor *qds) {
	DNode_t* node = &qnodes[queryId];
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

void init() {
	initDocumentDescriptorPool();
	initLinkedListDefaultPool();
	lazy_list = newLinkedList();
	queries = newLinkedList();
	int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	printf("%d\n", numCPU);
	eltire = newTrie3();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////

LinkedList_t* lists[NUM_THREADS_DOC][1];
long long doc_word_num[NUM_THREADS_DOC];

void *matcher_thread(void *n) {
	int tid = (uintptr_t) n;
#ifdef THREAD_ENABLE
	while (1) {
#endif
		DocumentDescriptor *doc_desc = (DocumentDescriptor *) cir_queue_remove(
				&cirq_busy_docs);
		char *doc = doc_desc->document;
		int i = 0;
		int matchCount = 0;
		while (doc[i]) {
			while (doc[i] == ' ')
				i++;
			int e = i;
			while (doc[e] != ' ' && doc[e] != '\0')
				e++;

			doc_word_num[tid]++;

			long long time = TriewordExist(dtrie[tid], &doc[i], e - i,
					doc_desc->docId, lists[tid]);
			if (!time) {
				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
						eltire, lists[tid][0], 0, doc_word_num[tid]);
			} else {
				if (time > 0) {
					matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount,
							eltire, lists[tid][0], time, doc_word_num[tid]);
					DNode_t* cur = lists[tid][0]->head.next;
					while (cur != &(lists[tid][0]->tail)) {

					}
				}
			}
			i = e;
		}

		doc_desc->matches = (QueryID *) malloc(sizeof(QueryID) * matchCount);
		doc_desc->numResults = matchCount;

		i = 0;
		int p = 0;

		DNode_t* cur = queries->head.next;
		while (cur != &(queries->tail)) {
			QueryDescriptor * cqd = (QueryDescriptor *) cur->data;
			if (cqd->docId[tid] == doc_desc->docId
					&& cqd->matchedWords[tid] == (1 << (cqd->numWords)) - 1)
				doc_desc->matches[p++] = cqd->queryId;
			if (p == matchCount)
				break;
			cur = cur->next;
		}

		//XXX could be moved above when we're using array instead of linkedlist
		cir_queue_insert(&cirq_free_docs, doc_desc->document);

		pthread_mutex_lock(&docList_lock);
		doc_desc->next = docList.next;
		docList.next = doc_desc;
		pthread_cond_signal(&docList_avail);
		pthread_mutex_unlock(&docList_lock);
		//pthread_mutex_unlock(&big_debug_lock);
#ifdef THREAD_ENABLE
	}
#endif
	return 0;
}

ErrorCode InitializeIndex() {
	init();
	docCount = 0;
	cir_queue_init(&cirq_free_docs, (void **) &free_docs, CIR_QUEUE_SIZE);
	cir_queue_init(&cirq_busy_docs, (void **) &busy_docs, CIR_QUEUE_SIZE);
	cir_queue_init(&cirq_busy_queries, (void **) &busy_queries, CIR_QUEUE_SIZE);
	cir_queue_init(&cirq_free_segments, (void **) &free_segments,
	CIR_QUEUE_SIZE);
	cir_queue_init(&cirq_busy_segments, (void **) &busy_segments,
	CIR_QUEUE_SIZE);

	pthread_mutex_init(&big_debug_lock, NULL );
	pthread_mutex_init(&trie_lock, NULL );
	pthread_mutex_init(&docList_lock, NULL );
	pthread_cond_init(&docList_avail, NULL );

	int i;
	for (i = 0; i < NUM_THREADS_DOC; i++) {
		//dyn_array_init(&matches[i], RES_POOL_INITSIZE);
		dtrie[i] = newTrie2();
	}

	for (i = 0; i < CIR_QUEUE_SIZE; i++) {
		free_docs[i] = documents[i];
	}

	for (i = 0; i < QDESC_MAP_SIZE ; i++)
		edit_list[i] = newLinkedList();

	cirq_free_docs.size = CIR_QUEUE_SIZE;
	cirq_free_segments.size = CIR_QUEUE_SIZE;

#ifdef THREAD_ENABLE
	for (i = 0; i < NUM_THREADS_DOC; i++) {
		pthread_create(&matcher_threads[i], NULL, matcher_thread,
				(void *) (uintptr_t) i);
	}
	for (i = 0; i < NUM_THREADS_QUERY; i++) {
		pthread_create(&candidate_gen_threads[i], NULL, generate_candidates,
				(void *) (uintptr_t) i);
	}
#endif
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
	//printf("%d\n", cntz);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

//void *lazyStart(void *n) {
//	int tid = (uintptr_t) n;
//#ifdef THREAD_ENABLE
//	while (1) {
//#endif
//	QueryDescriptor* queryDescriptor = cir_queue_remove(cirq_busy_docs);
void lazyStart(QueryDescriptor* queryDescriptor) {
#ifdef THREAD_ENABLE
	waitTillFull(&cirq_free_docs);
#endif
	int in, match_type = queryDescriptor->matchType, numOfWords =
			queryDescriptor->numWords, query_id = queryDescriptor->queryId,
			match_dist = queryDescriptor->matchDistance, iq = 0, wordLength, i,
			j, s;
	int numOfSegments = match_dist + 1;
	for (in = 0; in < numOfWords; in++) {
		SegmentData *sd = &(queryDescriptor->segments[in]);
		sd->parentQuery = queryDescriptor;
		sd->queryId = query_id;
		sd->wordIndex = in;
		cir_queue_remove(&cirq_free_segments);
		cir_queue_insert(&cirq_busy_segments, sd);
#ifndef THREAD_ENABLE
		generate_candidates(0);
#endif
//			generate_candidates(queryDescriptor->words[in],
//					queryDescriptor->words[in + 1] - queryDescriptor->words[in],
//					match_dist, sd);
	}
//		continue;
	return;
//#ifdef THREAD_ENABLE
//}
//#endif
//	return 0;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {

//TODO DNode_t ** segmentsData ;
#ifdef THREAD_ENABLE
	//waitTillFull(&cirq_free_docs);
#endif
	int in = 0, i = 0, j = 0, wordLength = 0, iq = 0, s;

	int wordSizes[6];
	int numOfWords = 0;
	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = &qmap[query_id];
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;
	queryDescriptor->queryId = query_id;
	for (in = 0; in < NUM_THREADS_DOC; in++)
		queryDescriptor->docId[in] = -1;

	addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryDescriptor, query_str, &numOfWords);
	queryDescriptor->numWords = numOfWords;

	DNode_t* lazy_node = append(lazy_list, queryDescriptor);
	lazy_nodes[query_id] = lazy_node;

	return EC_SUCCESS;

}

/*
 * this method take string and split it to subStrings(words) as the words are space separated
 */

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx) {
	int iq = 0;
	char *output = desc->queryString;
	char **words = desc->words;

	*idx = 0;
	int idx2 = 0;

	// if the string is null
	//TODO i think we must fire error
	if (!query_str[iq])
		return;
	// assume there are spaces in the first
	while (query_str[iq] && query_str[iq] == ' ')
		iq++;
	words[*idx] = output;
	int idx1 = 0;
	// loop and get the words
	while (query_str[iq]) {
		if (query_str[iq] == ' ') {
			while (query_str[iq] == ' ')
				iq++;
			if (query_str[iq]) {
				length[(*idx)] = idx1;
				(*idx)++;
				words[*idx] = &output[idx2];
				idx1 = 0;
			}
		}
		//to handle spaces in the end of query
		if (query_str[iq]) {
			output[idx2++] = query_str[iq];
			idx1++;
			iq++;
		}
	}
	length[(*idx)] = idx1;
	(*idx)++;
	words[*idx] = &output[idx2];
}

///////////////////////////////////////////////////////////////////////////////////////////////

int cnt5 = 0;
ErrorCode EndQuery(QueryID query_id) {
#ifdef THREAD_ENABLE
	waitTillFull(&cirq_free_segments);
	waitTillFull(&cirq_free_docs);
#endif

	QueryDescriptor* queryDescriptor = &qmap[query_id];
	removeQuery(query_id, queryDescriptor);
	if (lazy_nodes[query_id]) {
		DNode_t*tmp = lazy_nodes[query_id];
		delete_node(lazy_nodes[query_id]);
		lazy_nodes[query_id] = 0;
		removeQuery(query_id, queryDescriptor);
		return EC_SUCCESS;
	}

	LinkedList_t * list = edit_list[query_id];
	DNode_t *cur = list->head.next;
	while (cur != &(list->tail)) {
		DNode_t * node = (DNode_t *) cur->data;
		delete_node(node);
		cur = cur->next;
	}

	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////
int cmpfunc(const QueryID * a, const QueryID * b) {
	return (*a - *b);
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	DNode_t* lazy_node = lazy_list->head.next, *tmp;

	while (lazy_node != &(lazy_list->tail)) {
		tmp = lazy_node->next;
		lazyStart((QueryDescriptor*) (lazy_node->data));
		lazy_nodes[((QueryDescriptor*) (lazy_node->data))->queryId] = 0;
		delete_node(lazy_node);
		lazy_node = tmp;
	}

#ifdef THREAD_ENABLE
	waitTillFull(&cirq_free_segments);
#endif
	docCount++;
	char *doc_buf = (char *) cir_queue_remove(&cirq_free_docs);
	strcpy(doc_buf, doc_str);
	DocumentDescriptor *desc = newDocumentDescriptor();
	desc->docId = doc_id;
	desc->document = doc_buf;
	cir_queue_insert(&cirq_busy_docs, desc);
#ifndef THREAD_ENABLE
	matcher_thread(0);
#endif
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	if (docCount == 0)
		return EC_NO_AVAIL_RES;
	pthread_mutex_lock(&docList_lock);
	while (docList.next == 0)
		pthread_cond_wait(&docList_avail, &docList_lock);
	DocumentDescriptor* doc_desc = docList.next;
	docList.next = doc_desc->next;
	pthread_mutex_unlock(&docList_lock);

	docCount--;
	*p_query_ids = doc_desc->matches;
	*p_doc_id = doc_desc->docId;
	*p_num_res = doc_desc->numResults;
	if (doc_desc->numResults == 0)
		free(doc_desc->matches);
	dealloc_docDesc(doc_desc);
	return EC_SUCCESS;
}
char result[NUM_THREADS_QUERY][300000][33];
int lengths[NUM_THREADS_QUERY][300000];
int indexxx[NUM_THREADS_QUERY][300000];
//int maxLen[NUM_THREADS] = 0;

//void generate_candidates(char * str, int len, int dist, SegmentData* segData) {
void *generate_candidates(void *n) {
	int tid = (uintptr_t) n;
#ifdef THREAD_ENABLE
	while (1) {
#endif
		SegmentData* segData = (SegmentData *) cir_queue_remove(
				&cirq_busy_segments);
		char *str = segData->parentQuery->words[segData->wordIndex];
		int len = segData->parentQuery->words[segData->wordIndex + 1]
				- segData->parentQuery->words[segData->wordIndex];
		int dist = segData->parentQuery->matchDistance;
		int type = segData->parentQuery->matchType;
		if (type == MT_EXACT_MATCH)
			dist = 0;
		int start = 0, end = 1;
		lengths[tid][0] = len;
		int i;
		for (i = 0; i < len; i++)
			result[tid][0][i] = str[i];
		int id = 0;
		int resIndex = 1;
		indexxx[tid][0] = 0;
		while (dist > 0) {
			for (id = start; id < end; id++) {
				str = result[tid][id];
				int i, j;
				// insert
				for (i = indexxx[tid][id]; i < lengths[tid][id]; i++) {
					int ptr = 0;
					if (type == MT_EDIT_DIST) {
						for (j = 0; j < i; j++)
							result[tid][resIndex][ptr++] = str[j];
						result[tid][resIndex][ptr++] = lamda;
						for (j = i; j < lengths[tid][id]; j++)
							result[tid][resIndex][ptr++] = str[j];
						result[tid][resIndex][ptr] = '\0';
						lengths[tid][resIndex] = lengths[tid][id] + 1;
						indexxx[tid][resIndex] = i + 1;
						resIndex++;
						// delete
						ptr = 0;
						for (j = 0; j < i; j++)
							result[tid][resIndex][ptr++] = str[j];
						for (j = i + 1; j < lengths[tid][id]; j++)
							result[tid][resIndex][ptr++] = str[j];
						result[tid][resIndex][ptr] = '\0';
						lengths[tid][resIndex] = lengths[tid][id] - 1;
						indexxx[tid][resIndex] = i;
						resIndex++;
					}
					// swap
					ptr = 0;
					for (j = 0; j < lengths[tid][id]; j++)
						result[tid][resIndex][ptr++] = str[j];
					result[tid][resIndex][i] = lamda;
					result[tid][resIndex][ptr] = '\0';
					lengths[tid][resIndex] = lengths[tid][id];
					indexxx[tid][resIndex] = i + 1;
					resIndex++;
				}
				if (type == MT_EDIT_DIST) {
					int ptr = 0;
					for (j = 0; j < i; j++)
						result[tid][resIndex][ptr++] = str[j];
					result[tid][resIndex][ptr++] = lamda;
					for (j = i; j < lengths[tid][id]; j++)
						result[tid][resIndex][ptr++] = str[j];
					result[tid][resIndex][ptr] = '\0';
					lengths[tid][resIndex] = lengths[tid][id] + 1;
					indexxx[tid][resIndex] = i + 1;
					resIndex++;
				}
			}
			start = end;
			end = resIndex;
			dist--;
		}
#ifndef CONC_TRIE3
		pthread_mutex_lock(&trie_lock);
#endif
		int ind = start;
		while (ind < end) {
			DNode_t * node = InsertTrie3(eltire, result[tid][ind],
					lengths[tid][ind], segData);
			sync_append(edit_list[segData->queryId], node);
			ind++;
		}
#ifndef CONC_TRIE3
		pthread_mutex_unlock(&trie_lock);
#endif
		cir_queue_insert(&cirq_free_segments, NULL );
#ifdef THREAD_ENABLE
	}
#endif
	return 0;
}
///////////////////////////////////////////
void core_test() {
	InitializeIndex();
	char f[32] = "air";
	char f2[32] = "aix";

	StartQuery(5, f, MT_EDIT_DIST, 1);
	MatchDocument(10, "air");
	EndQuery(5);
	puts("====");
	fflush(0);

//	DocID did;
//	QueryID *qid;
//	unsigned int numRes;
//	GetNextAvailRes(&did, &numRes, &qid);
//	int i;
//	for (i = 0; i < numRes; i++)
//		printf("---->%d\n", qid[i]);
//	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
}
