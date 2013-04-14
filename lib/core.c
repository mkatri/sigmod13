//#define CORE_DEBUG
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

typedef struct {
	DocumentDescriptor *task_docs[DOC_PER_TASK];
	int numTasks;
} MatchTask;

MatchTask curTask;

long long overhead;
long long total;
int cmpfunc(const void* a, const void* b);
void generate_candidates(SegmentData* segData);
///////////////////////////////////////////////////////////////////////////////////////////////
//Trie_t *trie;
Trie_t2 * dtrie;
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

/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

LinkedList_t qresult;

void init() {
	initDocumentDescriptorPool();
	initLinkedListDefaultPool();
	lazy_list = newLinkedList();
	queries = newLinkedList();
//	trie = newTrie();
	eltire = newTrie3();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////
int cnt = 0;

inline long bsfl2(long bitmask) {
	long first = 0;
	long isZero = -1;
	asm(
			"bsf %1, %0\n\t"
			"cmove %2, %0"
			:"=r"(first)
			:"g"(bitmask), "rm"(isZero)
			:);

	return first;
}

void do_match() {
	int matchCount[DOC_PER_TASK];
	MatchTask *task = &curTask;
	DocumentDescriptor **doc_desc_array = curTask.task_docs;
	int task_size = task->numTasks;
	int d;
	memset(matchCount, 0, sizeof(int) * DOC_PER_TASK);

	threaded_matchTrie(doc_desc_array[0]->docId, matchCount, task_size, &(dtrie->root),
			&(eltire->root), &qresult);

	int p[DOC_PER_TASK];
	for (d = 0; d < task_size; d++) {
		doc_desc_array[d]->matches = (QueryID *) malloc(
				sizeof(QueryID) * matchCount[d]);
		doc_desc_array[d]->numResults = matchCount[d];
		p[d] = 0;
	}

	int w;
	long queryStatus;

	DNode_t* cur = qresult.head.next;
	while (cur != &(qresult.tail)) {
		QueryDescriptor *qdesc = (QueryDescriptor *) cur->data;
		queryStatus = qdesc->thSpec.docsMatchedWord[0];
		for (w = 1; w < qdesc->numWords; w++)
			queryStatus &= qdesc->thSpec.docsMatchedWord[w];

		long d;
		while ((d = bsfl2(queryStatus)) > -1) {
			queryStatus ^= (1L << d);
			doc_desc_array[d]->matches[p[d]++] = qdesc->queryId;
		}
		cur = delete_node(cur);
	}

	for (d = 0; d < task_size; d++) {
		qsort(doc_desc_array[d]->matches, matchCount[d], sizeof(QueryID),
				cmpfunc);
	}

	for (d = 0; d < task_size; d++) {
		doc_desc_array[d]->next = docList.next;
		docList.next = doc_desc_array[d];
	}

	task->numTasks = 0;
}

ErrorCode InitializeIndex() {
	init();
	docCount = 0;

	dtrie = newTrie2();
	qresult.head.next = &(qresult.tail), qresult.tail.prev = &(qresult.head);
	initialize_matchTrie();
	int i;
	for (i = 0; i < QDESC_MAP_SIZE ; i++)
		edit_list[i] = newLinkedList();
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
	printf("\n%lld total iterations\n", total);
	printf("\n%lld overhead iterations: \n\n", overhead);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void lazyStart(QueryDescriptor* queryDescriptor) {
	int in, numOfWords = queryDescriptor->numWords, query_id =
			queryDescriptor->queryId;
	for (in = 0; in < numOfWords; in++) {
		SegmentData *sd = &(queryDescriptor->segments[in]);
		sd->parentQuery = queryDescriptor;
		sd->queryId = query_id;
		sd->wordIndex = in;
		generate_candidates(sd);
	}
	return;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {

	if (curTask.numTasks != 0) {
		do_match();
	}

	int wordSizes[6];
	int numOfWords = 0;
	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = &qmap[query_id];
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;
	queryDescriptor->queryId = query_id;

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
	if (lazy_nodes[query_id]) {
		delete_node(lazy_nodes[query_id]);
		lazy_nodes[query_id] = 0;
		return EC_SUCCESS;
	}

	if (curTask.numTasks != 0) {
		do_match();
	}

	LinkedList_t * list = edit_list[query_id];
	DNode_t *cur = list->head.next;
	while (cur != &(list->tail)) {
		DNode_t * node = (DNode_t *) cur->data;
		delete_node(node);
		cur = delete_node(cur);
	}
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////
int cmpfunc(const void* a, const void* b) {
	return (*((QueryID *) a) - *((QueryID *) b));
}

void insertDocument(DocumentDescriptor *doc_desc, const char *document_buffer) {
	int d = curTask.numTasks;
	long fingerprint = 1L << d;
	int i = 0;
	while (document_buffer[i]) {
		while (document_buffer[i] == ' ')
			i++;
		int e = i;
		while (document_buffer[e] != ' ' && document_buffer[e] != '\0')
			e++;
		TrieDocInsert(dtrie, &document_buffer[i], e - i,
				curTask.task_docs[0]->docId, fingerprint);
		i = e;
	}
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	DNode_t* lazy_node = lazy_list->head.next;

	while (lazy_node != &(lazy_list->tail)) {
		lazyStart((QueryDescriptor*) (lazy_node->data));
		lazy_nodes[((QueryDescriptor*) (lazy_node->data))->queryId] = 0;
		lazy_node = delete_node(lazy_node);
	}

	docCount++;
	DocumentDescriptor *desc = newDocumentDescriptor();
	desc->docId = doc_id;
	curTask.task_docs[curTask.numTasks] = desc;
	insertDocument(desc, doc_str);
	//KEEP THE FOLLOWING LINE IN IT's PLACE

	if (++(curTask.numTasks) == DOC_PER_TASK) {
		do_match();
	}

	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	if (docCount == 0)
		return EC_NO_AVAIL_RES;

	if (curTask.numTasks != 0) {
		do_match();
	}

	DocumentDescriptor* doc_desc = docList.next;
	docList.next = doc_desc->next;

	docCount--;
	*p_query_ids = doc_desc->matches;
	*p_doc_id = doc_desc->docId;
	*p_num_res = doc_desc->numResults;
	//TODO
	if (doc_desc->numResults == 0)
		free(doc_desc->matches);
	dealloc_docDesc(doc_desc);
	return EC_SUCCESS;
}
char result[300000][33];
int lengths[300000];
int indexxx[300000];
int lastOperation[300000];
//int maxLen[NUM_THREADS] = 0;

void generate_candidates(SegmentData* segData) {
	char *str = segData->parentQuery->words[segData->wordIndex];
	int len = segData->parentQuery->words[segData->wordIndex + 1]
			- segData->parentQuery->words[segData->wordIndex];
	int dist = segData->parentQuery->matchDistance;
	int type = segData->parentQuery->matchType;
	if (type == MT_EXACT_MATCH)
		dist = 0;
	int start = 0, end = 1;
	lengths[0] = len;
	int i;
	for (i = 0; i < len; i++)
		result[0][i] = str[i];
	int id = 0;
	int resIndex = 1;
	indexxx[0] = 0;
	while (dist > 0) {
		for (id = start; id < end; id++) {
			str = result[id];
			int i, j;
			char test = (lastOperation[id] == 1 ? 1 : 0);
			for (i = indexxx[id]; i < lengths[id]; i++) {
				int ptr = 0;
				if (type == MT_EDIT_DIST) {
					if (test)
						i++;
					if (i < lengths[id]) {
						// insert
						for (j = 0; j < i; j++)
							result[resIndex][ptr++] = str[j];
						result[resIndex][ptr++] = lamda;
						for (j = i; j < lengths[id]; j++)
							result[resIndex][ptr++] = str[j];
						result[resIndex][ptr] = '\0';
						lengths[resIndex] = lengths[id] + 1;
						indexxx[resIndex] = i + 1;
						lastOperation[resIndex] = 0;
						resIndex++;
					}
					if (test)
						i--;
					// delete
					ptr = 0;
					for (j = 0; j < i; j++)
						result[resIndex][ptr++] = str[j];
					for (j = i + 1; j < lengths[id]; j++)
						result[resIndex][ptr++] = str[j];
					result[resIndex][ptr] = '\0';
					lengths[resIndex] = lengths[id] - 1;
					indexxx[resIndex] = i;
					lastOperation[resIndex] = 1;
					resIndex++;
				}
				if (i == lengths[id] - 1 && dist > 1)
					continue;

				// swap
				ptr = 0;
				for (j = 0; j < lengths[id]; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][i] = lamda;
				result[resIndex][ptr] = '\0';
				lengths[resIndex] = lengths[id];
				indexxx[resIndex] = i + 1;
				lastOperation[resIndex] = 2;
				resIndex++;
			}
			if (type == MT_EDIT_DIST) {
				int ptr = 0;
				for (j = 0; j < i; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][ptr++] = lamda;
				for (j = i; j < lengths[id]; j++)
					result[resIndex][ptr++] = str[j];
				result[resIndex][ptr] = '\0';
				lengths[resIndex] = lengths[id] + 1;
				indexxx[resIndex] = i + 1;
				lastOperation[resIndex] = 0;
				resIndex++;
			}
		}
		start = end;
		end = resIndex;
		dist--;
	}
	int ind = start;
	while (ind < end) {
		DNode_t * node = InsertTrie3(eltire, result[ind], lengths[ind],
				segData);
		if (node)
			append(edit_list[segData->queryId], node);
		ind++;
	}
	return;
}
///////////////////////////////////////////
void core_test() {
	InitializeIndex();
	char f[32] = "abcd";
//	char f2[32] = "aix";

	StartQuery(5, f, MT_EDIT_DIST, 3);
	MatchDocument(10, "s");
	EndQuery(5);
	puts("====");
	fflush(0);

	DocID did;
	QueryID *qid;
	unsigned int numRes;
	GetNextAvailRes(&did, &numRes, &qid);
	int i;
	for (i = 0; i < numRes; i++)
		printf("---->%d\n", qid[i]);
	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
}
