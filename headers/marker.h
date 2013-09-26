#ifndef __GS_MARKER_H__
	#define __GS_MARKER_H__
	/* Fun Fact: GS stands for green-serv*/

	#define GS_MARKER_INVALID_ID -1
	#define GS_MARKER_CREATED_TIME_LENGTH 19
	
	typedef struct{			 /* Calling them more convenient terms: */
		long left;  		 /* characteristic */
		unsigned long right; /* mantissa */
	}Decimal;

	void createDecimal(long left, unsigned long right, Decimal * dec);

	struct gs_marker {
		long id;
		long commentId;
		long scopeId;
		char createdTime[GS_MARKER_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
		Decimal latitude;
		Decimal longitude;
	};

	/* Any functions specifically working with just gs_marker: */
	void gs_marker_setId(long id, struct gs_marker * gsm);
	void gs_marker_setCommentId(long id, struct gs_marker * gsm);
	void gs_marker_setScopeId(long ScopeId, struct gs_marker * gsm);
	void gs_marker_setCreatedTime(char * createdTime, struct gs_marker * gsm);
	void gs_marker_setLongitude(Decimal longitude, struct gs_marker * gsm);
	void gs_marker_setLatitude(Decimal latitude, struct gs_marker * gsm);
	
	/* For Invalid Flags (Perhaps they should be moved to their own header...)*/
	#include "scope.h"
	#include "comment.h"

	/* Empties a marker structure of data and sets flag values */
	void gs_marker_ZeroStruct(struct gs_marker * gsm);

	#include "db.h" /* For pagination and invalid flag*/

	#define gs_marker_GET_ALL "SELECT id, commentId, scope_id, created_time, latitude, longitude FROM markers WHERE scope_id = %ld ORDER BY created_time DESC LIMIT %d, " STRINGIFY(RESULTS_PER_PAGE) ";"
	#define gs_marker_GET_BY_ID "SELECT id, commentId, scope_id, created_time, latitude, longitude FROM markers WHERE id = %ld;"
	#define gs_marker_INSERT "INSERT INTO markers (commentId, latitude, longitude) VALUES (%ld, %ld.%lu, %ld.%lu);"




#endif
