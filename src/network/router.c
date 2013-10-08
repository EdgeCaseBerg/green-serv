#include "network/router.h"

static inline int min(int a, int b){
	return a < b ? a : b;
}

int determineController(char * url, int urlLength){
	/* Compare only the length up to the point where parameters might
	 * show up. This is not a regex after all. (although we could prob
	 *-ably use strstr...)
	*/
urlLength+=1;
	if(strncasecmp(url, "/api/comments",13 ) == 0		||
			strncasecmp(url,"/api/comments/",14 ) == 0	){
			/* Comments Controller */
			return 	COMMENTS_CONTROLLER;
	}else if(strncasecmp(url, "/api/heatmap",12 ) == 0	||
			strncasecmp(url, "/api/heatmap/",13 ) == 0	){
			return HEATMAP_CONTROLLER;
	}else if(strncasecmp(url,"/api/pins",9 ) == 0		||
			strncasecmp(url,"/api/pins/",10 ) == 0		){
			return MARKER_CONTROLLER;
	}else if(strncasecmp(url,"/api/debug",10 ) == 0 	||
			strncasecmp(url,"/api/debug/",11 ) == 0		){
			return REPORT_CONTROLLER;
	}else if(strcasecmp(url, "") == 0 		||
			strcasecmp(url, "/" ) == 0		||
			strcasecmp(url, "/api/" ) == 0	||
			strcasecmp(url, "/api" ) == 0	){
			/* HeartBeat Controller */
			return HEARTBEAT_CONTROLLER;
	}/* No Route matches. */
	return INVALID_CONTROLLER;
}