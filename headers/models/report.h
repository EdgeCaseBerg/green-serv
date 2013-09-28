#ifndef __GS_REPORT_H__
	#define __GS_REPORT_H__
	/* Fun Fact: GS stands for green-serv*/

	#include "flags.h"
	#define GS_REPORT_MAX_LENGTH 1024
	#define GS_REPORT_CREATED_TIME_LENGTH 19
	#define SHA_LENGTH 64 /* 64 * 4 = 256*/
	
	struct gs_report {
		long id;
		long scopeId;
		char content[GS_REPORT_MAX_LENGTH+1]; /* 140 chars + \0 = 141 */
		char origin[SHA_LENGTH+1];
		char authorize[SHA_LENGTH+1];
		char createdTime[GS_REPORT_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
	};

	/* Any functions specifically working with just gs_report: */
	void gs_report_setId(long id, struct gs_report * gsr);
	/* Content will be truncated to be within GS_REPORT_MAX_LENGTH characters plus a null character!*/
	void gs_report_setContent(char * content, struct gs_report * gsr);
	void gs_report_setScopeId(long ScopeId, struct gs_report * gsr);
	void gs_report_setCreatedTime(char * createdTime, struct gs_report * gsr); /* Contemplate long for epoch time? */
	void gs_report_setOrigin(char * origin, struct gs_report * gsr);
	void gs_report_setAuthorize(char * auth, struct gs_report * gsr);

	/* Empties a report structure of data and sets flag values */
	void gs_report_ZeroStruct(struct gs_report * gsr);




#endif