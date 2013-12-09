#ifndef __M_LIST_H__
	#define __M_LIST_H__

	#include <stdlib.h>

	struct mNode {
		struct mNode * next;
		void * data;
	};

	/* Returns a pointer to the inserted node for chaining 
	 * Pass NULL for head to instantiate head node.
	 * Calling party is expected to free memory for nodes. Unless destroy list used
	 * The *data should be allocated on the heap becuase destroy list will attempt
	 * to free it. If this behavior is not desired, then implement your own 
	 * destroy list function and define the flag NO_DSTRY_MLIST before including
	 * the mlist.h file in your code.
	*/
	struct mNode * create_node(struct mNode * head, void * data); 

	/* Frees memory used by node list */
	void destroy_list(struct mNode * head);


#endif