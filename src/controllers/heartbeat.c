#include "controllers/heartbeat.h"

/*Respond to a Get Request. 
 * Takes the string to fill with the response
 * and the length of allocation
*/
int heartbeat_get(char * stringToReturn, int strLength){
	/* Retrieve Heartbeat */
	char json[GET_RESPONSE_JSON_SIZE];
	bzero(json, GET_RESPONSE_JSON_SIZE);
	gs_heartbeatNToJSON(json,GET_RESPONSE_JSON_SIZE);

	/* Copy response into the string to be returned */
	if ( strLength < GET_RESPONSE_JSON_SIZE ) {
		fprintf(stderr, "%s\n", "Possible truncation of Heartbeat JSON");
		return snprintf(stringToReturn, strLength, "%s", json);
	} else {
		return sprintf(stringToReturn,"%s",json);
	}
} 