#include "../include/core.h"

typedef struct {
	char *document;
	/* TODO of any use?
	 char *nextWord;
	 */
	DocID docId;
	QueryID *matches;
	unsigned int numResults;

} DocumentDescriptor;
