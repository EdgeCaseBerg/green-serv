#include "controllers/markers.h"

int marker_controller(const struct http_request * request, char * stringToReturn, int strLength){
	fprintf(stderr, "Working with %p %s %d", (void*)request, stringToReturn, strLength);
	int status;
	int buffSize;
	int numParams;
	long id; 
	char * buffer; 
	char tempBuf[40];
	char **convertSuccess;
	Decimal * latDegrees;
	Decimal * lonDegrees;
	Decimal * latOffset;
	Decimal * lonOffset;
	StrMap * sm;

	status = 503;
	convertSuccess = NULL;
	buffSize = (MARKER_LIMIT * sizeof(struct gs_marker))*4+1+(2*MAX_URL_LENGTH);
	bzero(tempBuf, sizeof tempBuf);

	/*Buffer up a good size that will probably not get filled (hopefully)*/
	buffer = malloc(buffSize);
	if(buffer == NULL){
		status = 500;
		goto mc_nomem;
	}
	buffer = memset(buffer,0,buffSize);

	latDegrees = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer);
		status = 500;
		goto mc_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}
	lonDegrees = malloc(sizeof(Decimal));
	if (lonDegrees == NULL) {
		free(buffer); free(latDegrees);		
		status = 500;
		goto mc_nomem;
	} else {
		lonDegrees = memset(lonDegrees,0,sizeof(Decimal));
	}
	latOffset = malloc(sizeof(Decimal));
	if (latOffset == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees);
		status = 500;
		goto mc_nomem;
	} else {
		latOffset = memset(latOffset,0,sizeof(Decimal));
	}
	lonOffset = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset);
		status = 500;
		goto mc_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
		goto mc_nomem;
	}

	numParams = parseURL(request->url, strlen(request->url), sm);
	if(numParams > 0){
		/* Collect any parameters and convert them to the proper types */
		if (sm_exists(sm,"latdegrees")==1) {
			sm_get(sm,"latdegrees",tempBuf,sizeof tempBuf);
			createDecimalFromString(latDegrees, tempBuf);
			/* Offsets only make sense if their parent coordinate is existent */
			if (sm_exists(sm,"latoffset")==1) {
				sm_get(sm,"latoffset", tempBuf, sizeof tempBuf);
				/* Validate the numericness of the offset */
				if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL)
					createDecimalFromString(latOffset,tempBuf);
				else{
					sm_delete(sm);
					free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
					goto mc_badLatOffset;
				} 
			} else {
				createDecimalFromString(latOffset,DEFAULT_OFFSET);
			}
		} else{
			/* Sensible default for latdegrees is... to become NULL 
			 * so that we can handle using a different query without 
			 * relying on magic numbers of any kind. 
			*/
			free(latDegrees);
			latDegrees = NULL;
		}
		if (sm_exists(sm,"londegrees")==1) {
			sm_get(sm,"lonDegrees",tempBuf,sizeof tempBuf);
			createDecimalFromString(lonDegrees, tempBuf);
			if (sm_exists(sm,"lonoffset")==1) {
				sm_get(sm,"lonoffset", tempBuf, sizeof tempBuf);
				if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL)
					createDecimalFromString(lonOffset,tempBuf);
				else{
					sm_delete(sm);
					free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
					goto mc_badLonOffset;
				} 
			} else {
				createDecimalFromString(lonOffset,DEFAULT_OFFSET);
			}
		} else {
			free(lonDegrees);
			lonDegrees = NULL;
		}


	}
	/* Check Method and fly off the handle */
	switch(request->method){
		case GET:
			break;
		case POST:
			break;
		case PUT:
			if(sm_exists(sm,"id")!=1){
				status = 400;
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_missing_key;
			}
			sm_get(sm, "id", tempBuf, sizeof tempBuf);
			id = atol(tempBuf);
			status = marker_address(buffer,buffSize,id,request);
			break;
		case DELETE:
			if(sm_exists(sm,"id")!=1){
				status = 400;
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_missing_key;
			}
			sm_get(sm, "id", tempBuf, sizeof tempBuf);
			id = atol(tempBuf);
			status = marker_delete(buffer, buffSize, id);
			if(status == -1){
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_nomem;	
			}
			break;
		default:
			status = 501;	
			free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
			sm_delete(sm);
			goto mc_unsupportedMethod;
	}

	snprintf(stringToReturn, strLength, "%s", buffer);
	free(buffer); 
	free(latDegrees); 
	free(lonDegrees); 
	free(latOffset); 
	free(lonOffset);
	sm_delete(sm);	
	return status;

	mc_nomem:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, 500, NOMEM_ERROR);
		return status;

	mc_unsupportedMethod:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_METHOD_ERR);
		return status;		
	
	mc_missing_key:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, MISSING_ID_KEY);
		return status;	

	mc_badLonOffset:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_LON_OFFSET);
		return status;		
	
	mc_badLatOffset:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_LAT_OFFSET);
		return status;		


}

int marker_delete(char * buffer, int buffSize, long id){
	MYSQL *conn;
	long affected; 

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	affected = db_deleteMarker(id,conn);

	mysql_close(conn);
	mysql_thread_end();

	if(affected > 0){
		snprintf(buffer,buffSize,"{\"status_code\" : 204,\"message\" : \"Successfuly deleted marker\"}");
		return 204;
	} else {
		snprintf(buffer,buffSize,"{\"status_code\" : 404,\"message\" : \"Could not find marker with given id\"}");
		return 404;
	}
}

int marker_address(char * buffer, int buffSize, long id, const struct http_request * request){
	MYSQL *conn;
	long affected; 
	int addressed;
	char * charP; 
	char * boolVal;

	affected = addressed =  0;
	fprintf(stderr, "%ld %ld\n", affected, id);

	/* Retrieve the put data first */
	if(request->contentLength > 0){
		charP = strstr(request->data, "addressed");
		if(charP == NULL){
			snprintf(buffer, buffSize,ERROR_STR_FORMAT,422,NO_ADDRESSED_KEY);
			return 422;
		} else {
			/* Key was found, skip the field name and find the value*/
			fprintf(stderr, "%s\n", charP);
			charP+=9;
			fprintf(stderr, "%s\n", charP);
			boolVal = strstr(charP, "true");
			if(boolVal == NULL){
				boolVal = strstr(charP, "false");
				if(boolVal == NULL){
					snprintf(buffer, buffSize,ERROR_STR_FORMAT,422,NO_ADDRESSED_KEY);
					return 422;	
				}else{
					addressed = ADDRESSED_FALSE;
				}	
			}else{
				addressed = ADDRESSED_TRUE;
			}
		}
	}else{
		/* No put data bad request */
		snprintf(buffer, buffSize, ERROR_STR_FORMAT,400, NO_PUT_DATA);
		return 400;
	}
	

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	/* Mark addressed */	
	affected = db_addressMarker(id, addressed, conn);	

	mysql_close(conn);
	mysql_thread_end();

	if(affected > 0){
		snprintf(buffer, buffSize, "\"status_code\" : 200,\"message\":\"successful update\"");
	}else{
		snprintf(buffer,buffSize,"{\"status_code\" : 404,\"message\" : \"Could not find marker with given id\"}");
		return 404;
	}

	return 200;
}