#include <stdlib.h>
#include "helpers/mlist.h"
#include "models/comment.h"
#include "models/marker.h"
#include "models/heatmap.h"
#include <unistd.h>


int main() {
	struct mNode * head;
	struct mNode * tmp;
	struct mNode * second;
	struct gs_comment * cData;
	struct gs_marker * mData;
	struct gs_heatmap * hData;

	cData = malloc(sizeof (struct gs_comment));
	if(cData == NULL)
		goto out_of_mem;

	mData = malloc(sizeof (struct gs_marker));
	if(mData == NULL){
		free(cData);
		goto out_of_mem;
	}
	hData = malloc(sizeof (struct gs_heatmap));
	if(hData == NULL){
		free(cData); free(mData);
		goto out_of_mem;
	}

	head = NULL;
	head = create_node(head, (void *)cData);
	if(head == NULL){
		free(cData); free(mData); free(hData);
		goto out_of_mem;
	}

	fprintf(stdout, "Created Head at: %p\n", (void*)head);
	fprintf(stdout, "Expect head data to be: %p, it is: %p \n", (void*)cData, (void*)head->data);

	second = create_node(head, (void *)mData);
	if(second == NULL){
		destroy_list(head);
		free(mData);
		free(hData);
		goto out_of_mem;
	}

	fprintf(stdout, "Head next should point to tmp, head->next: %p and tmp: %p \n", (void*)head->next, (void*)second); 
	fprintf(stdout, "tmp data should be mData tmp->data: %p, mData: %p \n", (void*)second->data, (void*)mData );

	tmp = create_node(head, (void *)hData);
	if(tmp == NULL){
		destroy_list(head);
		free(hData);
		goto out_of_mem;
	}

	fprintf(stdout, "head->next->next should now be tmp: head: %p, tmp: %p \n",(void*) second->next, (void*)tmp);
	fprintf(stdout, "tmp data should be hData tmp->data: %p, hData: %p \n", (void*)tmp->data, (void*)hData );

	fprintf(stdout, "Destroying list... ");
	destroy_list(head);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return 0;

	out_of_mem:
		fprintf(stderr, "Couldn't do test out of memory\n");
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return 1;
}