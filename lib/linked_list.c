#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>

LinkedList_t* newLinkedList() {
	LinkedList_t* ret = (LinkedList_t*) malloc(sizeof(LinkedList_t));
	ret->head.next = &(ret->tail), ret->tail.prev = &(ret->head);
	ret->head.data = 0, ret->tail.data = 0;
	ret->head.prev = 0, ret->tail.next = 0;
	return ret;
}
DNode_t* append(LinkedList_t* list, void * data) {
	DNode_t* node = (DNode_t *) (malloc(sizeof(DNode_t)));
	node->prev = list->tail.prev, node->next = &(list->tail);
	node->next->prev = node, node->prev->next = node;
	node->data = data;
	return node;
}
DNode_t* delete_node(DNode_t * node) {
	node->next->prev = node->prev;
	node->prev->next = node->next;
	DNode_t* nxt = node->next;
	free(node);
	return nxt;
}
char isEmpty(LinkedList_t * list) {
	return list == 0 || list->head.next == &list->tail;
}
//int main(int argc, char **argv) {
//	LinkedList_t *lst = newLinkedList();
//	int a = 5, b = 7, c = 8;
//	append(lst, &a);
//	append(lst, &b);
//	append(lst, &c);
//	append(lst, &b);
//	append(lst, &a);
//	DNode_t* current = (lst->head.next);
//	while (current->next) {
//		printf("%d\n", *((int*) current->data)); // *((int*) current.data));
//		if (*((int*) current->data) == 7) {
//			current = delete(current);
//		} else
//			current = current->next;
//	}
//	puts("hello people");
//	current = (lst->head.next);
//	while (current->next) {
//		printf("%d\n", *((int*) current->data)); // *((int*) current.data));
//		if (*((int*) current->data) == 7) {
//			current = delete(current);
//		} else
//			current = current->next;
//	}
//	return 0;
//}

