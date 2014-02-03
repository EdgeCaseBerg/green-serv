#ifndef __GS_REPORT_H__
	#define __GS_REPORT_H__
	/* Fun Fact: GS stands for green-serv*/

	#include "flags.h"
	#define GS_REPORT_MAX_LENGTH 512
	#define GS_REPORT_CREATED_TIME_LENGTH 19
	#define GS_REPORT_TYPE_MAX_LENGTH GS_REPORT_CREATED_TIME_LENGTH
	#define SHA_LENGTH 64 /* 64 * 4 = 256*/
	#define REPORT_TYPES "AUTH,INFO,DEBUG,WARN,ERROR"
	
	struct gs_report {
		long id;
		long scopeId;
		char content[GS_REPORT_MAX_LENGTH+1];
		char trace[GS_REPORT_MAX_LENGTH+1];
		char origin[SHA_LENGTH+1];
		char authorize[SHA_LENGTH+1];
		char createdTime[GS_REPORT_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
		char rType[GS_REPORT_TYPE_MAX_LENGTH+1]; 
	};

	/* setOrigin, and setAuthorize both perform sha256 calculations */

	/* Any functions specifically working with just gs_report: */
	void gs_report_setId(long id, struct gs_report * gsr);
	/* Content will be truncated to be within GS_REPORT_MAX_LENGTH characters plus a null character!*/
	void gs_report_setContent(char * content, struct gs_report * gsr);
	void gs_report_setStackTrace(char * trace, struct gs_report * gsr);
	void gs_report_setScopeId(long ScopeId, struct gs_report * gsr);
	void gs_report_setCreatedTime(char * createdTime, struct gs_report * gsr); /* Contemplate long for epoch time? */
	void gs_report_setOrigin(char * origin, struct gs_report * gsr);
	void gs_report_setAuthorize(char * auth, struct gs_report * gsr);
	void gs_report_setType(char * type, struct gs_report * gsr);

	/* Empties a report structure of data and sets flag values */
	void gs_report_ZeroStruct(struct gs_report * gsr);

	#include "helpers/sha256.h"
	#include <string.h>
	
	


#endif