#ifndef __GS_COMMENT_H__
	#define __GS_COMMENT_H__
	/* Fun Fact: GS stands for green-serv*/

	#define GS_COMMENT_INVALID_ID -1
	#define GS_COMMENT_MAX_LENGTH 140
	#define GS_COMMENT_CREATED_TIME_LENGTH 19
	
	struct gs_comment {
		long id;
		char content[GS_COMMENT_MAX_LENGTH+1]; /* 140 chars + \0 = 141 */
		long scopeId;
		char createdTime[GS_COMMENT_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
	};

	/* Any functions specifically working with just gs_comment: */
	void gs_comment_setId(long id, struct gs_comment * gsc);
	/* Content will be truncated to be within 140 characters plus a null character!*/
	void gs_comment_setContent(char * content, struct gs_comment * gsc);
	void gs_comment_setScopeId(long ScopeId, struct gs_comment * gsc);
	void gs_comment_setCreatedTime(char * createdTime, struct gs_comment * gsc); /* Contemplate long for epoch time? */
	

	/* Empties a comment structure of data and sets flag values */
	void gs_comment_ZeroStruct(struct gs_comment * gsc);

	#include "db.h" /* For pagination and invalid flag*/

	#define GS_COMMENT_GET_ALL "SELECT id, content, scope_id, created_time FROM comments LIMIT %d, " STRINGIFY(RESULTS_PER_PAGE) ";"
	#define GS_COMMENT_GET_BY_ID "SELECT id, content, scope_id, created_time FROM comments WHERE id = %ld;"
	#define GS_COMMENT_INSERT "INSERT INTO comments (content, scope_id) VALUES (\"%s\", %ld);"



#endif