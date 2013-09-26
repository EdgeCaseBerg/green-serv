
#include "scope.h" /* For invalid id for scope */
#include "comment.h"

#include <string.h>


void gs_comment_setId(long id, struct gs_comment * gsc){
	gsc->id = id;
}

void gs_comment_setContent(char * content, struct gs_comment * gsc){
	bzero(gsc->content,GS_COMMENT_MAX_LENGTH+1);
	strncpy(gsc->content,content,GS_COMMENT_MAX_LENGTH);
}

void gs_comment_setScopeId(long scopeId, struct gs_comment * gsc){
	gsc->scopeId = scopeId;
}

void gs_comment_setCreatedTime(char * createdTime, struct gs_comment * gsc){
	bzero(gsc->createdTime, GS_COMMENT_CREATED_TIME_LENGTH+1);
	strncpy(gsc->createdTime, createdTime, GS_COMMENT_CREATED_TIME_LENGTH);
}

/* Clears out a scope structure to make it ready for use.*/
void gs_comment_ZeroStruct(struct gs_comment * gsc){
	bzero(gsc->content,GS_COMMENT_MAX_LENGTH+1);
	gsc->id = GS_COMMENT_INVALID_ID; 
	gsc->scopeId = GS_SCOPE_INVALID_ID;
	bzero(gsc->createdTime, GS_COMMENT_CREATED_TIME_LENGTH+1);
}