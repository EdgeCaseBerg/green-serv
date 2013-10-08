#include "network/router.h"

static inline int min(int a, int b){
	return a < b ? a : b;
}

int determineController(char * url, int urlLength){
	/* Compare only the length up to the point where parameters might
	 * show up. This is not a regex after all. (although we could prob
	 *-ably use strstr...)
	*/
	if(strncasecmp(url, "/api/comments",min(13,urlLength))			||
			strncasecmp(url,"/api/comments/",min(14,urlLength))	){
			/* Comments Controller */
			return 	COMMENTS_CONTROLLER;
	}else if(strncasecmp(url, "/api/heatmap",min(12,urlLength))	||
			strncasecmp(url, "/api/heatmap/",min(13,urlLength))	){
			return HEATMAP_CONTROLLER;
	}else if(strncasecmp(url,"/api/pins",min(9,urlLength))			||
			strncasecmp(url,"/api/pins/",min(10,urlLength))		){
			return MARKER_CONTROLLER;
	}else if(strncasecmp(url,"/api/debug",min(10,urlLength)) 		||
			strncasecmp(url,"/api/debug/",min(11,urlLength))		){
			return REPORT_CONTROLLER;
	}else if(strncasecmp(url, "",min(1,urlLength)) 		||
			strncasecmp(url, "/",min(1,urlLength))		||
			strncasecmp(url, "/api/",min(5,urlLength))	||
			strncasecmp(url, "/api", min(4,urlLength))	){
			/* HeartBeat Controller */
			return HEARTBEAT_CONTROLLER;
	}/* No Route matches. */
	return INVALID_CONTROLLER;
}