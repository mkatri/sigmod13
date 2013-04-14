#include <core.h>

typedef struct DocumentDescriptor {
	union {
		struct {
			char *document;
			DocID docId;
			QueryID *matches;
			unsigned int numResults;
			struct DocumentDescriptor *next;
		};
		char padding[64];
	};
} DocumentDescriptor;

void initDocumentDescriptorPool();
DocumentDescriptor *newDocumentDescriptor();
void dealloc_docDesc(DocumentDescriptor *desc);
