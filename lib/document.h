#include <core.h>

typedef struct DocumentDescriptor {
	char *document;
	/* TODO of any use?
	 char *nextWord;
	 */
	DocID docId;
	QueryID *matches;
	unsigned int numResults;
	struct DocumentDescriptor *next;
} DocumentDescriptor;

void initDocumentDescriptorPool();
DocumentDescriptor *newDocumentDescriptor();
void dealloc_docDesc(DocumentDescriptor *desc);
