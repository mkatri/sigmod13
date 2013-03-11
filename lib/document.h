#include <core.h>

typedef struct {
  /* TODO of any use?
	char *document;
	char *nextWord;
	*/
	DocID docId;
	QueryID *matches;
	unsigned int numResults;

} DocumentDescriptor;
