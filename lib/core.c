/*
 * core.cpp version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Author: Amin Allam
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

//#define CORE_DEBUG
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <core.h>
#include "query.h"
#include "trie.h"
#include "document.h"
#include "Hash_Table.h"
#include "cir_queue.h"
#include "submit_params.h"
#include "dyn_array.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////// DOC THREADING STRUCTS //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

pthread_t threads[NUM_THREADS];
char documents[NUM_THREADS][MAX_DOC_LENGTH];
CircularQueue cirq_free_docs;
CircularQueue cirq_busy_docs;
char *free_docs[NUM_THREADS];
DocumentDescriptor *busy_docs[NUM_THREADS];
pthread_mutex_t docList_lock;
pthread_cond_t docList_avail;
//DynamicArray matches[NUM_THREADS];
int cmpfunc(const QueryID * a, const QueryID * b);
///////////////////////////////////////////////////////////////////////////////////////////////

Trie_t *trie;
LinkedList_t *docList;
//LinkedList_t *queries;
unsigned long docCount;

/*QUERY DESCRIPTOR MAP GOES HERE*/
QueryDescriptor *qmap;
//HashTable* ht;
Trie_t2 *dtrie[NUM_THREADS];
//inline QueryDescriptor * getQueryDescriptor(int queryId) {
//	return qmap[queryId];
//}
/*
 inline void addQuery(int queryId, QueryDescriptor * qds) {
 //	qmap[queryId] = qds;

 DNode_t* node = append(queries, qds);
 insert(ht, queryId, node);

 }
 */
/*QUERY DESCRIPTOR MAP ENDS HERE*/

void split(int length[6], QueryDescriptor *desc, const char* query_str,
		int * idx);

void init() {
//	queries = newLinkedList();
//	ht = new_Hash_Table();

	qmap = malloc(QDESC_MAP_SIZE * sizeof(QueryDescriptor));
	trie = newTrie();
	docList = newLinkedList();
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries

///////////////////////////////////////////////////////////////////////////////////////////////

void *matcher_thread(void *n) {
	int tid = n;
	while (1) {
		DocumentDescriptor *doc_desc = cir_queue_remove(&cirq_busy_docs);
		char *doc = doc_desc->document;
		int i = 0;
		int matchCount = 0;
		while (doc[i]) {
			while (doc[i] == ' ')
				i++;

			int e = i;
			while (doc[e] != ' ' && doc[e] != '\0')
				e++;

			if (!TriewordExist(dtrie[tid], &doc[i], e - i, doc_desc->docId)) {
				TrieInsert2(dtrie[tid], &doc[i], e - i, doc_desc->docId);
				matchWord(doc_desc->docId, tid, &doc[i], e - i, &matchCount);
			}
			i = e;
		}

		doc_desc->matches = malloc(sizeof(QueryID) * matchCount);
		doc_desc->numResults = matchCount;

		/*
		 memcpy(doc_desc->matches, qres, sizeof(QueryID) * matches[tid].tail);
		 qsort(doc_desc->matches, matches[tid].tail, sizeof(QueryID), cmpfunc);
		 */

		i = 0;
		int p = 0;
		/*
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
		 */
		while (i < QDESC_MAP_SIZE) {
			QueryDescriptor * cqd = &qmap[i++];
			if (cqd->docId[tid] == doc_desc->docId
					&& cqd->matchedWords[tid] == (1 << (cqd->numWords)) - 1) {
				if (cqd->queryId == i - 1)
					printf("it really is fucked up\n");
				doc_desc->matches[p++] = i - 1;
			}
			if (p == matchCount)
				break;
		}
		//XXX could be moved above when we're using array instead of linkedlist
		cir_queue_insert(&cirq_free_docs, doc_desc->document);

		pthread_mutex_lock(&docList_lock);
		append(docList, doc_desc);
		pthread_cond_signal(&docList_avail);
		pthread_mutex_unlock(&docList_lock);
	}
	return 0;
}

ErrorCode InitializeIndex() {
	init();
	docCount = 0;
	cir_queue_init(&cirq_free_docs, &free_docs, NUM_THREADS);
	cir_queue_init(&cirq_busy_docs, &busy_docs, NUM_THREADS);

	pthread_mutex_init(&docList_lock, NULL );
	pthread_cond_init(&docList_avail, NULL );

	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		free_docs[i] = documents[i];
		//dyn_array_init(&matches[i], RES_POOL_INITSIZE);
		dtrie[i] = newTrie2();
	}
	cirq_free_docs.size = NUM_THREADS;

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, matcher_thread, i);
	}
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex() {
//	/printf("%d\n", cnt);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void printWords(char out[6][32], int num) {
#ifdef CORE_DEBUG
	int i = 0;
	for (i = 0; i < num; i++)
	puts(out[i]);
#endif
}

ErrorCode StartQuery(QueryID query_id, const char* query_str,
		MatchType match_type, unsigned int match_dist) {
//#ifdef CORE_DEBUG
//	printf("query: %d --> %s\n", query_id, query_str);
//#endif

//TODO DNode_t ** segmentsData ;
	waitTillFull(&cirq_free_docs);
	int in = 0, i = 0, j = 0, wordLength = 0, k, first, second, iq = 0;

	int wordSizes[6];
	int numOfWords = 0;
	int numOfSegments = match_dist + 1;
	//get query descriptor for the query
	QueryDescriptor * queryDescriptor = &qmap[query_id];
	queryDescriptor->matchDistance = match_dist;
	queryDescriptor->matchType = match_type;
	queryDescriptor->queryId = query_id;
	assert(queryDescriptor->queryId == query_id);
	for (in = 0; in < NUM_THREADS; in++)
		queryDescriptor->docId[in] = -1;

	//addQuery(query_id, queryDescriptor);

	//as the query words are space separated so this method return the words and it's length
	split(wordSizes, queryDescriptor, query_str, &numOfWords);
	queryDescriptor->numWords = numOfWords;
//	return 0;

	char segment[32];
	/*initialize the DNode array here*/
	int top = 0;
	/*
	 * XXX

	 queryDescriptor->segmentsData = (DNode_t**) malloc(
	 numOfSegments * numOfWords * sizeof(DNode_t *));
	 for (top = 0; top < numOfSegments * numOfWords; top++)
	 queryDescriptor->segmentsData[top] = 0;
	 */
	top = 0;
	//printf("num of words %d\n",numOfWords);
	for (in = 0; in < numOfWords; in++) {
		//get the word length
		iq = 0;
		wordLength = wordSizes[in];
		//printf("word >> %s\n", queryDescriptor->words[in]);
		//here (wordSizes[in]+1 to add the null at the end of char array

		/*
		 * k here as teste paper mention to get good partition with hamming 1
		 * example : assume word length =10 and distance=3
		 * so we partition the word to 4  segments with length(3,3,2,2)
		 * so first =3;
		 * and second =2;
		 */
		/*how do we prove this*/
		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
		first = (wordLength + numOfSegments - 1) / numOfSegments;
		second = wordLength / numOfSegments;
		// loop on the word to get the segments
		for (i = 0; i < k; i++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->startIndex = queryDescriptor->words[in] + iq;
			sd->segmentIndex = i;
			for (j = 0; j < first; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}

			segment[j] = '\0';
			//load the segment data
			sd->queryId = query_id;
			//sd->startIndex = iq - first;
			sd->wordIndex = in;

			//insert in trie
			//	printf("segment >>>> %s\n", segment);
			sd->trieNode = TrieInsert(trie, segment, first, match_type, sd);
			queryDescriptor->segmentsData[top++][i] = sd;
		}

		// loop on the word to get the segments
		for (i = 0; (i < numOfSegments - k) && second; i++) {
			SegmentData *sd = newSegmentdata();
			sd->parentQuery = queryDescriptor;
			sd->startIndex = queryDescriptor->words[in] + iq;
			sd->segmentIndex = i + k;
			for (j = 0; j < second; j++) {
				segment[j] = *(queryDescriptor->words[in] + iq);
				iq++;
			}
			segment[j] = '\0';
			//load segments data
			sd->queryId = query_id;
			//sd->startIndex = iq - second;
			sd->wordIndex = in;
			//insert in trie
			//	printf("segment >>>> %s\n", segment);
			sd->trieNode = TrieInsert(trie, segment, second, match_type, sd);
			queryDescriptor->segmentsData[top++][i] = sd;
		}
	}

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

ErrorCode EndQuery(QueryID query_id) {
#ifdef CORE_DEBUG
	puts("inside here");
#endif

//	QueryDescriptor* queryDescriptor = getQueryDescriptor(query_id);
	waitTillFull(&cirq_free_docs);
	QueryDescriptor* queryDescriptor = &qmap[query_id];
	int i, j;
	int in, iq, wordLength, numOfSegments = queryDescriptor->matchDistance + 1,
			k, first, second;
	char segment[32];
	int top = 0;
	for (in = 0; in < 5 && queryDescriptor->words[in + 1]; in++) {

		//get the word length
		iq = 0;
		wordLength = queryDescriptor->words[in + 1]
				- queryDescriptor->words[in];
		//here (wordSizes[in]+1 to add the null at the end of char array
		/*
		 * k here as teste paper mention to get good partition with hamming 1
		 * example : assume word length =10 and distance=3
		 * so we partition the word to 4  segments with length(3,3,2,2)
		 * so first =3;
		 * and second =2;
		 */
		/*how do we prove this*/
#ifdef CORE_DEBUG
		printf(">>>>>     %d %d\n", wordLength, numOfSegments);
#endif

		k = wordLength - (wordLength / numOfSegments) * (numOfSegments);
		first = (wordLength + numOfSegments - 1) / numOfSegments;
		second = wordLength / numOfSegments;
		// loop on the word to get the segments
		for (i = 0; i < k; i++) {
			for (j = 0; j < first; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top][k]->trieNode); //TODO ALSO DELETE SEGMENT DATA inside the node
			free(queryDescriptor->segmentsData[top][k]);
			top++;
			//Delete from the trie
			TrieDelete(trie, segment, first, queryDescriptor->matchType);
		}

		// loop on the word to get the segments
		for (i = 0; (i < numOfSegments - k) && second; i++) {
			for (j = 0; j < second; j++) {
				segment[j] = queryDescriptor->words[in][iq];
				iq++;
			}
			segment[j] = '\0';

			//Delete from the linked list in trie nodes
			delete(queryDescriptor->segmentsData[top][k]->trieNode); //TODO ALSO DELETE SEGMENT DATA inside the node
			free(queryDescriptor->segmentsData[top][k]);
			top++;
			//Delete from the trie
			TrieDelete(trie, segment, second, queryDescriptor->matchType);
		}
	}
//	freeQueryDescriptor(queryDescriptor);
//	delete_H(ht, query_id);
//	node = (DNode_t*) get(ht, query_id);
//	qmap[query_id] = 0;
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int cmpfunc(const QueryID * a, const QueryID * b) {
	return (*a - *b);
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
	docCount++;
	char *doc_buf = cir_queue_remove(&cirq_free_docs);
	strcpy(doc_buf, doc_str);
	DocumentDescriptor *desc = malloc(sizeof(DocumentDescriptor));
	desc->docId = doc_id;
	desc->document = doc_buf;
	cir_queue_insert(&cirq_busy_docs, desc);
	return EC_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res,
		QueryID** p_query_ids) {
	if (docCount == 0)
		return EC_NO_AVAIL_RES;
	pthread_mutex_lock(&docList_lock);
	while (isEmpty(docList))
		pthread_cond_wait(&docList_avail, &docList_lock);
	DNode_t *node = docList->head.next;
	DocumentDescriptor* doc_desc = (DocumentDescriptor *) (node->data);
	delete(node);
	pthread_mutex_unlock(&docList_lock);

	docCount--;
	*p_query_ids = doc_desc->matches;
	*p_doc_id = doc_desc->docId;
	*p_num_res = doc_desc->numResults;
	if (doc_desc->numResults == 0)
		free(doc_desc->matches);
	free(doc_desc);
	return EC_SUCCESS;
}

///////////////////////////////////////////
void core_test() {

//	unsigned int t = 9113677439;
//	printf("%llu",t); fflush(0);
//	printf("%d\n\n", sizeof(HashCluster));
//	printf("%d\n\n", sizeof(int));
//	printf("%d\n\n", sizeof(HashCluster*));
	InitializeIndex();
//	char output[32][32];
//

	char f[32] = "diocese pgdma";
//	char f2[32] = "  ok no   fucker  ";
//
//	StartQuery(5, f, 0, 7);
	StartQuery(7, f, MT_HAMMING_DIST, 2);
//
//	dfs(&(trie->root));
//	EndQuery(7);
//	dfs(&(trie->root));
//	return;
//	printf("done\n");

	//hashTest();
	MatchDocument(10,
			"http dbpedia resource list women national basketball association players http dbpedia ontology abstract this list women national basketball association players active former current players bold also list national basketball association players basketball contents edit tajama abraham svetlana abrosimova jessica adair danielle adams jordan adams elisa aguilar matee ajavon marcie alberts markita aldridge erin alexander tawona alhaleem charel allen mactabene amachree monique ambers ambrosia anderson chantelle anderson jolene anderson keisha anderson mery andrade yvette angel nicky anosike jayne appel janeth arcain katasha artis marlies askamp tangela atkinson morenike atunrase seimone augustus angela aycock miranda ayim leigh aziz jennifer azzi edit sherill baker alison bales elena baranova adia barnes mistie bass suzy batkovic brown jacqueline batteast ashley battle cass bauer bilodeau alana beard tully bevilaqua jessica bibby agnieszka bibrzycka bird abby bishop nina bjedov angie bjorklund tera bjorklund chante black debbie black rhonda blades cindy blodgett nikki blue octavia blue shannon bobbitt whitney boddie ruthie bolton latoya bond dewanna bonner jenny boucek lindsay bowen carla boyd janice braxton kara braxton angie braziel jessica breland michelle brogan sandy brondello cindy brown kiesha brown jessica brungo rebekkah brunson vicky bullett heather burge heidi burge alisa burras janell burse tasha butts latasha byears edit elizabeth cambage edna campbell michelle campbell dominique canty monique cardenas jamie carey essence carson amisha carter deborah carter swin cash jamie cassidy iziane castro marques tamika catchings laneishea caufield elisabeth cebrian keri chaconas cori chambers quianna chaney jill chapman daily daedra charles tina charles sonia chase cheek felicia chester kaayla chones kayte christensen karima chritmas shameka christon kristi cirone margold clark michelle cleary stacy clinesmith claire coggins monique coker courtney coleman marissa coleman katrina colleton sydney colson megan compain andrea congreaves cara consuegra camille cooper cynthia cooper sylvia crawley willnett crockett danielle crockrom katie cronin shanna crossley cassandra crumpton moorer beth cunningham davalyn cunningham monique currie edniesha curry edit stacey dales schuman grace daley helen darling jessica davenport brandi davis davis clarissa davis wrightsil anna deforge jennifer derevjanik erika souza keitha dickerson dillard tamecka dixon nadine domond bethany donaphin shay doron cintia santos katie douglas megan duffy victoria dunlap candice dupree margo dydek edit michelle edwards simone edwards teresa edwards tonya edwards shyra shalonda enis summer lauren ervin edit trisha fallon barbara farris allison feaster sung marie ferdinand harris marta fernandez marina ferragut ukari figgs isabelle fijalkowski olga firsova milena flores fluker kristin folkl cheryl ford kisha ford stacey ford toni foster sylvia fowles desiree francis quonesia franklin megan frazee stacy frese keshia frett trina frierson linda frohlich candace futrell edit katryna gaither travesa gant begona garcia kerri gardin andrea gardner andrea garner pietra cornelia gayden katie gearlds gessig kelley gibson jennifer gillom usha gilmore kamela gissendanner chrissy givens emile gomis adrienne goodson bridgette gordon gillian goring shaunzinski gortman margo graham erin grant denique graves alexis gray lawson michelle greco kalana greene cisti greenwalt vedrana grgin fonseca yolanda griffith kelsey griffin lady grooms gordana grubin sandrine gruda wanda guyton edit mikiko hagiwara kamesha hairston amber hall vicki hall angie hamblin becky hammon hampton romana hamzova lindsey harding laura harper donna harrington amber harris fran harris lisa harrison kristi harrower vanessa hayden kristin haynie dena head nekeshia henderson tracy henderson sonja henning herrig katrina hibbert jessie hicks allison hightower hill korie hlede roneeka hodges ebony hoffman chamique holdsclaw kedra holland corn quanitra hollingsworth sequoia holmes holmes harris amber holt hope susie hopson shelton alexis hornbuckle charde houston ashley houts jennifer howard tasha humphrey edit ibekwe sandora irvin dalma ivanyi niele ivey edit angela jackson deanna jackson gwen jackson lauren jackson tamicha jackson tammy jackson jackson tiffany jackson amber jacobs tamara james briann january anete jekabsone zogota cathy joens pollyanna johns kimbrough adrienne johnson chandra johnson jaclyn johnson latonya johnson leslie johnson niesa johnson shannon johnson temeka johnson tiffani johnson vickie johnson kellie jolly harper asjha jones chandi jones jameka jones larecha jones marion jones merlakia jones pauline jordan edit aneta kausaite crystal kelly susan king borchardt kingi cross zuzi klimesova ewelina kobryn laurie koehn ilona korstin greta koss anastasia kostaki tanja kostic cathrine kraayeveld nicole kubik andrea kuklova edit alison lacey jennifer lacy natasha lacy venus lacy monica lamb sheila lambert merlelynn lange harris crystal langhorne krystyna lara erlana larkins amanda lassiter ivory latta jantel lavender kara lawson edwige lawson wade katarina lazic shalee lehning betty lennox lisa leslie yelena leuchanka nicole levandusky nicole levesque doneeka lewis takeisha lewis tynesha lewis nancy lieberman cline miao lijie taylor lilley camille little andrea lloyd curry rebecca lobo stacey lovelace luckey helen sancho lyttle edit mwadi mabika laura macchi clarisse machanguana megan mahoney shea mahoney hamchetou maiga nadine malcolm sonja mallory evanthia maltsi kristen mann sharon manning rhonda mapp michelle marciniak gabriela marginean maylana martin nuria martinez raffaella masciadri caity matter katie mattera anita maxwell monica maxwell kelly mazzante brandi mccain tiffany mccain rashanda mccants stephanie mccarty janel mccarville suzie mcconnell serio angel mccoughtry danielle mccray nikki mccray nicky mccrimmon danielle mcculley pamela mcgee carla mcghee mcwilliams franklin chasity melvin giuliana mendiola coco miller kelly miller teana miller tausha mills delisha milton jones leilani mitchell moeggenberg adriana moises jacinta monroe anna montanana alex montgomery renee montgomery jackie moore jessica moore loree moore maya moore navonda moore penny moore tamara moore yolanda moore carolyn moos jene morris bernice mosby judy mosley mcafee jenny mowe naomi mulitauaopele eshaya murphy edit chen christelle garsanet andrea nagy astou ndiaye diatta emmeline ndongue nemcova claudia neves chelsea newton bernadette ngoyisa tina nicholson marlous nieuwveen mila nikolich chioma nnamaka deanna nolan vanessa nygaard edit jenna kristen neill yuko nicole ohlde olajuwon irina osipova heather owen shantia owens josephine owino edit danielle page murriel page yolanda paige sabrina palie wendy palmer daniel courtney paris candace parker florina pascalau paschal michaela pavlickova kate paye kayla pedersen ticha penicheiro jocelyn penn jasmina perazic gipe perkins erin perperoglou perrot bridget pettis erin phillips porsha phillips tari phillips shia phillips plenette pierson jeanette pohlen catarina pollini cappie pondexter angie potthoff elaine powell nicole powell armintie price franthea price lynn pride epiphanny prince latoya pringle edit brooke queenan allie quigley noelle quinn texlin quinney edit hajdana radunovic felicia ragland semeka randall kristen rasmussen brittainey raven stephanie raymond tamika williams raymond jamie redd brandy reed chastity reed michelle reed tracy reid tammi reiss kathrin ress andrea riley ruth riley jennifer rizzotti nyree roberts ashley robinson crystal robinson danielle robinson renee robinson scholanda robinson jannon roland adrienne ross leah rush eugenia rycraw edit nykesha sales sheri charisse sampson isabel sanchez sanders nakia sanford olayinka sanni kelly santos alessandra santos oliveira rankica sarenac paige sauer jaynetta saunders audrey sauret laure savasta kelly schumacher georgia schweitzer olympia scott raegan scott elena shakirova sharp kristen sharp ashley shields adrienne shuler madinah slaise gwen slaughter gergana slavtcheva aiysha smith brooke smith charlotte smith charmin smith christy smith crystal smith jennifer smith katie smith smith lacharlotte smith tangela smith tyresa smith wanisha smith belinda snell michelle snow leila sobral sidney spencer rachael sporn trisha stafford odom dawn staley tiffany stansbury kate starbird katy steding maria stepanova rehema stephens stacy stephens mandisa stevenson jackie stiles valerie still andrea stinson shanele stires tamara stocks jurgita streimikyte strother tora suber laura summerton jung tammy sutton brown ketia swanier sheryl swoopes carolyn swords edit sonja tate diana taurasi lindsay taylor penny taylor nikki teasley zane teilane kasha terry carla thomas christi thomas jasmine thomas krystal thomas latoya thomas stacey thomas alicia thompson amanda thompson tina thompson shona thorburn erin thorn robin threatt iciss tillis michele timms penny toler kristi toliver elena tornikidou levys torres tiffany travis chantel tremitiere trena trice barbara turner molly tuter slobodanka tuvic polina tzekova edit mfon udoka petra ujhelyi itoro umoh coleman edit amaya valdemoro michele gorp courtney vandersloot alexandra vanembricqs vaughn krystal vaughn kristen veal jana vesela danielle viglione dalivorka vilipic kamila vodichkova natalia vodopyanova milica vukadinovic edit ashley walker ayana walker demya walker marcedes walker maren walseth coquese washington tonya washington wauters teresa weatherspoon umeki webb martina weber kendra wecker lindsay whalen detrina white erica white stephanie white white whiting raymond tamika whitmore khadijah whittington jennifer whittle wicks jamila wideman candice wiggins brittany wilkins angelina williams beverly williams debra williams williams natalie williams rita williams shaquala williams tara williams toccara williams adrian williams strong willingham lisa willis wendi willits amanda wilson christina wirth lindsay wisdom hylton sophia witherspoon kara wolters angelina wolvert lynette woodard tiffany woosley monica wright tanisha wright shereka wright brooke wyckoff dana wynne edit edit lindsey yamasaki corissa yasen nevriye yilmaz carolyn young sophia young tamera young edit oksana zakaluzhnaya francesca zara shavonte zellous haixia zheng zuzana zirkova http wikipedia wiki list women national basketball association players");
//	MatchDocument(10, "yomother fucker");
//	MatchDocument(20, "fuck you oknofutcher");
//	MatchDocument(30, "fuck mother you oknofucker father");
	DocID did;
	QueryID *qid;
	unsigned int numRes;
	GetNextAvailRes(&did, &numRes, &qid);

	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
//	GetNextAvailRes(&did, &numRes, &qid);
//	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
//	GetNextAvailRes(&did, &numRes, &qid);
//	printf("did = %d, first qid = %d, numRes = %d\n", did, qid[0], numRes);
////	EndQuery(0);
////	puts("---------------------------");
////	puts("---------------------------");
////	puts("---------------------------");
//
////	dfs(&(trie->root));
//	//	printo(f);
//	//	int num = 0;
//	//	getSegments(output, f, 11, 3, &num);
//	//	puts(output[3]);
//	//	printf("done %d", num);
}
