#include <string.h>
#include "models/heatmap.h"

/* Any functions specifically working with just gs_marker: */
void gs_mheatmap_setId(long id, struct gs_heatmap * gsh){
	gsh->id = id;
}

void gs_heatmap_setIntensity(long intensity, struct gs_heatmap * gsh){
	gsh->intensity = intensity;
}

void gs_heatmap_setScopeId(long scopeId, struct gs_heatmap * gsh){
	gsh->scopeId = scopeId;
}

/* Will truncate to 19 characters */
void gs_heatmap_setCreatedTime(char * createdTime, struct gs_heatmap * gsh){
	strncpy(gsh->createdTime, createdTime, GS_HEATMAP_CREATED_TIME_LENGTH);
}

void gs_heatmap_setLongitude(Decimal longitude, struct gs_heatmap * gsh){
	gsh->longitude = longitude;
}

void gs_heatmap_setLatitude(Decimal latitude, struct gs_heatmap * gsh){
	gsh->latitude = latitude;
}
	

/* Empties a marker structure of data and sets flag values */
void gs_heatmap_ZeroStruct(struct gs_heatmap * gsh){
	bzero(gsh->createdTime, GS_HEATMAP_CREATED_TIME_LENGTH+1);
	gsh->id = GS_HEATMAP_INVALID_ID;
	gsh->scopeId = GS_SCOPE_INVALID_ID;
	gsh->intensity = 0;
	/* 0 is actually a good 'invalid' or default for the right
	 * side because if someone gives just 1. or 1 to a string 
	 * parsing function and we only set the left, the right is 
	 * still correct!
	*/
	gsh->longitude.left = 0;  
	gsh->longitude.right = 0;
	gsh->latitude.left = 0;
	gsh->latitude.right = 0;
}
