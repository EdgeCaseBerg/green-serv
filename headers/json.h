#ifndef __JSON_H__
	#define __JSON_H__
	
	#include "scope.h" 

	int * _escapeJSON(char * input, int inputlen, char * output);
	int * gsScopeToJSON(struct gs_scope gss, char * jsonOutput);

#endif