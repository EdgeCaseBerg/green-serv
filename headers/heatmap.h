#ifndef __GS_HEATMAP_H__
	#define __GS_HEATMAP_H__

	#include "marker.h" /* For Decimal */
	#define GS_HEATMAP_CREATED_TIME_LENGTH 19

	struct gs_heatmap {
		long id;
		long scopeId;
		long intensity; /* "Seconds Worked" */
		char createdTime[GS_HEATMAP_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
		Decimal latitude;
		Decimal longitude;
	};

	void gs_heatmap_setId(long id, struct gs_heatmap * gsh);
	void gs_heatmap_setIntensity(long intensity, struct gs_heatmap * gsh);
	void gs_heatmap_setScopeId(long ScopeId, struct gs_heatmap * gsh);
	void gs_heatmap_setCreatedTime(char * createdTime, struct gs_heatmap * gsh);
	void gs_heatmap_setLongitude(Decimal longitude, struct gs_heatmap * gsh);
	void gs_heatmap_setLatitude(Decimal latitude, struct gs_heatmap * gsh);
	
	/* For Invalid Flags (Perhaps they should be moved to their own header...)*/
	#include "scope.h"

	/* Empties a marker structure of data and sets flag values */
	void gs_heatmap_ZeroStruct(struct gs_heatmap * gsh);

#endif