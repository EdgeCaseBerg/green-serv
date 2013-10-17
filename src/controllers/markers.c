#include "controllers/markers.h"

int marker_controller(const struct http_request * request, char * stringToReturn, int strLength){
	fprintf(stderr, "Working with %p %s %d", (void*)request, stringToReturn, strLength);
	int status;
	char * buffer; 
	char tempBuf[40];
	StrMap * sm;

	status = 503;
	
	bzero(tempBuf, sizeof tempBuf);

	/*Buffer up a good size that will probably not get filled (hopefully)*/
	buffer = malloc((MARKER_LIMIT * sizeof(struct gs_marker))*4+1+(2*MAX_URL_LENGTH));
	if(buffer == NULL){
		status = 500;
		goto mc_nomem;
	}
	buffer = memset(buffer,0,(MARKER_LIMIT * sizeof(struct gs_marker))*4+1+(2*MAX_URL_LENGTH));

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		free(buffer);
		goto mc_nomem;
	}

	/* Check Method and fly off the handle */
	switch(request->method){
		case GET:
			break;
		case POST:
			break;
		case PUT:
			break;
		case DELETE:
			break;
		default:
			status = 501;	
			free(buffer);
			sm_delete(sm);
			goto mc_unsupportedMethod;
	}

	free(buffer);
	sm_delete(sm);	
	return 503;

	mc_nomem:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, 500, NOMEM_ERROR);
		return status;

	mc_unsupportedMethod:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_METHOD_ERR);
		return status;		


}