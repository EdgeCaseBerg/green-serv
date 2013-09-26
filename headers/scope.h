
#ifndef __GS_SCOPE_H__
	#define __GS_SCOPE_H__
	/* Fun Fact: GS stands for green-serv*/

	#define GS_SCOPE_GET_ALL "SELECT id, description FROM scope LIMIT %d;"
	#define GS_SCOPE_GET_BY_ID "SELECT id, description FROM scope WHERE id = %ld;"
	#define GS_SCOPE_INSERT "INSERT INTO scope (description) VALUES (\"%s\");"

	#define GS_SCOPE_INVALID_ID -1
	

	struct gs_scope {
		long id;
		/* Normally we might use a pointer, but descriptions are always 8 chars*/
		char description[9]; /* 8 chars + \0 = 9 */
	};

	/* Any functions specifically working with just gs_scope: */
	void gs_scope_setId(long id, struct gs_scope * gss);

	/* Note that only 8 characters will ever be copied from desc*/
	void gs_scope_setDesc(char * desc, struct gs_scope * gss);

	/* Empties a scope structure of data and sets flag values */
	void gs_scope_ZeroStruct(struct gs_scope * gss);


#endif