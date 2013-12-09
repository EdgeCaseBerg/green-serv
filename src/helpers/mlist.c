#include "helpers/mlist.h"

struct mNode * create_node(struct mNode * head, void * data){
	struct mNode * nxt;
	struct mNode * tmp;

	if(head == NULL){
		nxt = malloc(sizeof (struct mNode));
		if(nxt == NULL){
			return NULL;
		}
		nxt->next = NULL;
		nxt->data = data;
	}else{
		nxt = malloc(sizeof (struct mNode));
		if(nxt == NULL){
			return NULL;
		}
		nxt->next = NULL;
		nxt->data = data;
		/* Iterate to position */
		for (tmp = head; tmp->next != NULL; tmp = tmp->next)
			;
		tmp->next = nxt;
	}
	return nxt;
}

/* See header file for why we do this. */
#ifndef NO_DSTRY_MLIST 
void destroy_list(struct mNode * head){
	if(head == NULL)
		return;

	destroy_list(head->next);
	/* We assume that the data passed to the void pointer
	 * was allocated with malloc function! Otherwise define NO_DSTRY_MLIST
	 * and create your own.
	*/
	free(head->data);
	head->data = NULL;
	free(head);
	head = NULL;
	return;
}
#endif