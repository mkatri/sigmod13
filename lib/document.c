/*
 * document.c
 *
 *  Created on: Apr 10, 2013
 *      Author: mkatri
 */
#include <stdlib.h>
#include <string.h>
#include "document.h"
#include "submit_params.h"

DocumentDescriptor *pool;
DocumentDescriptor reuse;
long poolSize;
long poolSpace;

void initDocumentDescriptorPool() {
	poolSize = INIT_DOCPOOL_SIZE;
	poolSpace = INIT_DOCPOOL_SIZE;
	reuse.next = 0;
	pool = (DocumentDescriptor *) malloc(poolSize * sizeof(DocumentDescriptor));
}

DocumentDescriptor *newDocumentDescriptor() {
	DocumentDescriptor *desc;
	if (reuse.next) {
		desc = reuse.next;
		reuse.next = desc->next;
	} else {
		if (poolSpace == 0) {
			pool = (DocumentDescriptor *) malloc(
					poolSize * sizeof(DocumentDescriptor));
			poolSpace = poolSize;
		}
		desc = pool;
		pool++;
		poolSpace--;
	}
	memset(desc, 0, sizeof(DocumentDescriptor));
	return desc;
}

void dealloc_docDesc(DocumentDescriptor *desc) {
	desc->next = reuse.next;
	reuse.next = desc;
}

