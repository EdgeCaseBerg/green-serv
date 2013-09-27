#ifndef __JSON_H__
	#define __JSON_H__
	
	#include "models/scope.h" 
	#include "models/comment.h"
	#include "models/marker.h"
	#include "models/heatmap.h"

	int _escapeJSON(const char * input, int inputlen, char * output);
	int gsScopeToJSON(const struct gs_scope gss, char * jsonOutput);
	int gsCommentToJSON(const struct gs_comment gsc, char * jsonOutput);
	int gsMarkerToJSON(const struct gs_marker gsm, char * jsonOutput);
	int gsHeatmapToJSON(const struct gs_heatmap gsh, char * jsonOutput);

#endif