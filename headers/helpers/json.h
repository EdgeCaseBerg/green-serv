#ifndef __JSON_H__
	#define __JSON_H__
	
	#include "models/scope.h" 
	#include "models/comment.h"
	#include "models/marker.h"
	#include "models/heatmap.h"
	#include "models/report.h"

	int _escapeJSON(const char * input, int inputlen, char * output);
	int gs_scopeToJSON(const struct gs_scope gss, char * jsonOutput);
	int gs_scopeNToJSON(const struct gs_scope gss, char * jsonOutput, int jsonOutputAllocatedSize);
	int gs_commentToJSON(const struct gs_comment gsc, char * jsonOutput);
	int gs_commentToNJSON(const struct gs_comment gsc, char * jsonOutput, int jsonOutputAllocatedSize);
	int gs_markerToJSON(const struct gs_marker gsm, char * jsonOutput);
	int gs_markerNToJSON(const struct gs_marker gsm, char * jsonOutput, int jsonOutputAllocatedSize);
	int gs_heatmapToJSON(const struct gs_heatmap gsh, char * jsonOutput);
	int gs_heatmapNToJSON(const struct gs_heatmap gsh, char * jsonOutput, int jsonOutputAllocatedSize);
	int gs_reportToJSON(const struct gs_report gsr, char * jsonOutput);
	int gs_reportNToJSON(const struct gs_report gsr, char * jsonOutput, int jsonOutputAllocatedSize);

#endif