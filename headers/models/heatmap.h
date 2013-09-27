#ifndef __GS_HEATMAP_H__
	#define __GS_HEATMAP_H__

	#include "flags.h"
	#include "models/marker.h" /* For Decimal type*/
	#define GS_HEATMAP_CREATED_TIME_LENGTH 19

	struct gs_heatmap {
		long id;
		long scopeId;
		long intensity; /* "Seconds Worked" */
		char createdTime[GS_HEATMAP_CREATED_TIME_LENGTH+1];  /* YYYY-MM-DD HH:MM:SS + \0 = 20*/
		Decimal latitude;
		Decimal longitude;
	};

	
	/* We hold these functions to be self evident
	 * that properly named functions need no documentation.
	 * That warnings and guidelines observed by their creator
	 * have un-ignorable sway over a developers usage. Among
	 * these are the right to read, ignore, and curse at the
	 * implementors.
	 * - The declaration of commenting well named functions
	*/
	void gs_heatmap_setId(long id, struct gs_heatmap * gsh);
	void gs_heatmap_setIntensity(long intensity, struct gs_heatmap * gsh);
	void gs_heatmap_setScopeId(long ScopeId, struct gs_heatmap * gsh);
	void gs_heatmap_setCreatedTime(char * createdTime, struct gs_heatmap * gsh);
	void gs_heatmap_setLongitude(Decimal longitude, struct gs_heatmap * gsh);
	void gs_heatmap_setLatitude(Decimal latitude, struct gs_heatmap * gsh);

	/* Empties a heatmap structure of data and sets flag values */
	void gs_heatmap_ZeroStruct(struct gs_heatmap * gsh);

#endif