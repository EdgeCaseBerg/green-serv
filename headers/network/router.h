#ifndef __ROUTER_H__
	#define __ROUTER_H__
	
	#define INVALID_CONTROLLER 0
	#define HEARTBEAT_CONTROLLER 1
	#define COMMENTS_CONTROLLER 2
	#define HEATMAP_CONTROLLER 3
	#define MARKER_CONTROLLER 4
	#define REPORT_CONTROLLER 5

	/* Returns a flag constant to determine which controller to call */
	int determineController(char * url);

	#include <string.h>
#endif