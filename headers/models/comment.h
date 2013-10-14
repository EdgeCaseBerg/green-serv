#ifndef __GS_COMMENT_H__
	#define __GS_COMMENT_H__
	/* Fun Fact: GS stands for green-serv*/

	#include "flags.h"
	#include "config.h"
	#define GS_COMMENT_MAX_LENGTH 140
	#define GS_COMMENT_CREATED_TIME_LENGTH 19
	#define GS_COMMENT_TYPE_LENGTH 10
	
	struct gs_comment {
		long id;
		long pinId; /*May be NULL */
		char content[GS_COMMENT_MAX_LENGTH+1]; /* 140 chars + \0 = 141 */
		char cType[GS_COMMENT_TYPE_LENGTH+1];
		long scopeId;
		char createdTime[GS_COMMENT_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
	};

	/* Any functions specifically working with just gs_comment: */
	void gs_comment_setId(long id, struct gs_comment * gsc);
	/* Content will be truncated to be within 140 characters plus a null character!*/
	void gs_comment_setContent(char * content, struct gs_comment * gsc);
	void gs_comment_setScopeId(long ScopeId, struct gs_comment * gsc);
	void gs_comment_setCreatedTime(char * createdTime, struct gs_comment * gsc); /* Contemplate long for epoch time? */
	void gs_comment_setPinId(long id, struct gs_comment * gsc);
	void gs_comment_setCommentType(char * cType, struct gs_comment * gsc);
	

	/* Empties a comment structure of data and sets flag values */
	void gs_comment_ZeroStruct(struct gs_comment * gsc);




#endif