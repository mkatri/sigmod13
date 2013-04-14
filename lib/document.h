#include <core.h>

typedef struct DocumentDescriptor {
	DocID docId;
	QueryID *matches;
	unsigned int numResults;
	struct DocumentDescriptor *next;
} DocumentDescriptor;

void initDocumentDescriptorPool();
DocumentDescriptor *newDocumentDescriptor();
void dealloc_docDesc(DocumentDescriptor *desc);
