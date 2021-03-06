#ifndef __GS_MARKER_H__
	#define __GS_MARKER_H__
	/* Fun Fact: GS stands for green-serv*/

    #include "flags.h"
	#define GS_MARKER_CREATED_TIME_LENGTH 19	
	#include "helpers/decimal.h"

	struct gs_marker {
		long id;
		long commentId;
		long scopeId;
		int addressed; /*API Version 3 addition. A pin may be 'addressed' if it has been 
						*taken care of in some way. In green-up this means that someone has
						*gone ahead and cleaned up an area someone might have marked as dirty.
					   */
		char createdTime[GS_MARKER_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
		Decimal latitude;
		Decimal longitude;
	};

	#ifndef ADDRESSED_TRUE
		#define ADDRESSED_TRUE 1
	#endif
	#ifndef ADDRESSED_FALSE
		#define ADDRESSED_FALSE 0
	#endif

	/* Any functions specifically working with just gs_marker: */
	void gs_marker_setId(long id, struct gs_marker * gsm);
	void gs_marker_setCommentId(long id, struct gs_marker * gsm);
	void gs_marker_setScopeId(long ScopeId, struct gs_marker * gsm);
	void gs_marker_setAddressed(int addressedState, struct gs_marker * gsm);
	void gs_marker_setCreatedTime(char * createdTime, struct gs_marker * gsm);
	void gs_marker_setLongitude(Decimal longitude, struct gs_marker * gsm);
	void gs_marker_setLatitude(Decimal latitude, struct gs_marker * gsm);
	

	/* Empties a marker structure of data and sets flag values */
	void gs_marker_ZeroStruct(struct gs_marker * gsm);     

#endif
