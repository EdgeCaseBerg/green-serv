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
	/* If anyone has a better idea for invalids I'm open to hearing it */
	gsm->longitude.left = 0;  
	gsm->longitude.right = 0;
	gsm->latitude.left = 0;
	gsm->latitude.right = 0;
}


/* Add two decimals */
void add_decimals(Decimal* a, Decimal* b, Decimal* sum){
    sum->left = a->left + b->left;
    sum->right = a->right + b->right;
    if(sum->right >= MANTISSA_LIMIT) {
        sum->left += 1;
        sum->right -= MANTISSA_LIMIT;
    }
}


/* Subtract two decimals, a - b */
void subtract_decimals(Decimal* a, Decimal* b, Decimal* diff){
    diff->left = a->left - b->left;
    diff->right = a->right - b->right;
    if(diff->right >= MANTISSA_LIMIT) { //it wll be around the limit for longs but it should never be lower than MANTISSA_LIMIT
        diff->left -= 1;                //because neither operand can be more than 1 billion
        diff->right = ULONG_MAX - diff->right;
    }
}

