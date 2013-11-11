#include "models/report.h"


void gs_report_setId(long id, struct gs_report * gsr){
	gsr->id = id;
}

void gs_report_setContent(char * content, struct gs_report * gsr){
	bzero(gsr->content,GS_REPORT_MAX_LENGTH+1);
	strncpy(gsr->content,content,GS_REPORT_MAX_LENGTH);
}

void gs_report_setStackTrace(char * trace, struct gs_report * gsr){
	bzero(gsr->trace,GS_REPORT_MAX_LENGTH+1);
	strncpy(gsr->trace,trace,GS_REPORT_MAX_LENGTH);	
}

void gs_report_setOrigin(char * origin, struct gs_report * gsr){
	bzero(gsr->origin,SHA_LENGTH+1);
	sha256(origin,gsr->origin);
}

void gs_report_setAuthorize(char * authorize, struct gs_report * gsr){
	bzero(gsr->authorize,SHA_LENGTH+1);
	sha256(authorize,gsr->authorize);
}

void gs_report_setScopeId(long scopeId, struct gs_report * gsr){
	gsr->scopeId = scopeId;
}

void gs_report_setCreatedTime(char * createdTime, struct gs_report * gsr){
	bzero(gsr->createdTime, GS_REPORT_CREATED_TIME_LENGTH+1);
	strncpy(gsr->createdTime, createdTime, GS_REPORT_CREATED_TIME_LENGTH);
}

/* Clears out a scope structure to make it ready for use.*/
void gs_report_ZeroStruct(struct gs_report * gsr){
	bzero(gsr->content,GS_REPORT_MAX_LENGTH+1);
	bzero(gsr->origin,SHA_LENGTH+1);
	bzero(gsr->authorize,SHA_LENGTH+1);
	bzero(gsr->trace, GS_REPORT_MAX_LENGTH +1);
	gsr->id = GS_REPORT_INVALID_ID; 
	gsr->scopeId = GS_SCOPE_INVALID_ID;
	bzero(gsr->createdTime, GS_REPORT_CREATED_TIME_LENGTH+1);
}