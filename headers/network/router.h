#ifndef __ROUTER_H__
	#define __ROUTER_H__
	
	#define INVALID_CONTROLLER 0
	#define HEARTBEAT_CONTROLLER 1
	#define COMMENTS_CONTROLLER 2
	#define HEATMAP_CONTROLLER 3
	#define MARKER_CONTROLLER 4
	#define REPORT_CONTROLLER 5
	#define HASH_TABLE_CAPACITY 15
	#define GET 2
	#define POST 4
	#define PUT 8
	#define DELETE 16
	#define UNKNOWN_METHOD 32
	#define ERROR_STR_FORMAT "{\"status_code\" : %i, \"Error_Message\": %s}"

	#include "helpers/strmap.h"
	#include <stdlib.h>
	#include <stdio.h>

	#define MAX_URL_LENGTH 100
	struct http_request{
		int method;
		char url[MAX_URL_LENGTH]; /* The request URL to determine the controller */
		int contentLength; /* The length of the content coming in */
		char * data; /* This must be malloced for data */
	};

	/* Returns a flag constant to determine which controller to call */
	int determineController(char * url);

	/* Parse a url and return the number of parameters begotten from it */
	int parseURL(const char * url, int urlLength, StrMap * table);

	#include <string.h>
#endif