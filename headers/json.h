#ifndef __JSON_H__
	#define __JSON_H__
	
	#include "scope.h" 
	#include "comment.h"

	int _escapeJSON(char * input, int inputlen, char * output);
	int gsScopeToJSON(struct gs_scope gss, char * jsonOutput);
	int gsCommentToJSON(struct gs_comment gsc, char * jsonOutput);
	int gsMarkerToJSON(struct gs_marker gsm, char * jsonOutput);

#endif