#include <string.h>
#include "models/marker.h"

/* Any functions specifically working with just gs_marker: */
void gs_marker_setId(long id, struct gs_marker * gsm){
	gsm->id = id;
}

void gs_marker_setCommentId(long id, struct gs_marker * gsm){
	gsm->commentId = id;
}

void gs_marker_setScopeId(long scopeId, struct gs_marker * gsm){
	gsm->scopeId = scopeId;
}

void gs_marker_setAddressed(int addressedState, struct gs_marker * gsm){
	if(addressedState != ADDRESSED_TRUE && addressedState != ADDRESSED_FALSE){
		fprintf(stderr, "%s\n", "Incorrect value for addressedState passed. Refusing to set.");
		return;
	}
	gsm->addressed = addressedState;
}

/* Will truncate to 19 characters */
void gs_marker_setCreatedTime(char * createdTime, struct gs_marker * gsm){
	strncpy(gsm->createdTime, createdTime, GS_MARKER_CREATED_TIME_LENGTH);
}

void gs_marker_setLongitude(Decimal longitude, struct gs_marker * gsm){
	gsm->longitude = longitude;
}

void gs_marker_setLatitude(Decimal latitude, struct gs_marker * gsm){
	gsm->latitude = latitude;
}
	

/* Empties a marker structure of data and sets flag values */
void gs_marker_ZeroStruct(struct gs_marker * gsm){
	bzero(gsm->createdTime, GS_MARKER_CREATED_TIME_LENGTH+1);
	gsm->id = GS_MARKER_INVALID_ID;
	gsm->commentId = GS_MARKER_INVALID_ID;
	gsm->addressed = ADDRESSED_FALSE;
	/* If anyone has a better idea for invalids I'm open to hearing it */
	gsm->longitude = 0;  
	gsm->latitude  = 0;
}


