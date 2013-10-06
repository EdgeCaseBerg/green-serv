#ifndef __HEARTBEAT_CONTROLLER_H__
	#define __HEARTBEAT_CONTROLLER_H__

	/*This controls the size allocated for the JSON of the heartbeat,
	 *tweak this for performance (it cannot go below a certain size)
	 *or you risk returning invalid JSON.
	*/
	#define GET_RESPONSE_JSON_SIZE 32	

	#include "helpers/json.h"
	#include "string.h"
	#include <stdio.h>

	/*Respond to a Get Request. 
 	* Takes the string to fill with the response
 	* and the length of allocation
	*/
	int heartbeat_get(char * stringToReturn, int strLength);
#endif