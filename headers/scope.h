
#ifndef __GS_SCOPE_H__
	#define __GS_SCOPE_H__
	/* Fun Fact: GS stands for green-serv*/

	#define GS_SCOPE_GET_ALL "SELECT id, description FROM scope;"
	#define GS_SCOPE_GET_BY_ID "SELECT id, description FROM scope WHERE id = %ld;"
	#define GS_SCOPE_GET_BY_DESC "SELECT id, description FROM scope where description = %8.8s"
	#define GS_SCOPE_INSERT "INSERT INTO scope (description) VALUES (%s);"

	struct gs_scope {
		long id;
		/* Normally we might use a pointer, but descriptions are always 8 chars*/
		char description[9]; /* 8 chars + \0 = 9 */
	};

	/* Any functions specifically working with just gs_scope: */


#endif