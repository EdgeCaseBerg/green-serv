#include "controllers/markers.h"

static inline int min(const int a, const int b){
	return a < b ? a : b;
}

/* Not inclusive */
static inline int between(const Decimal var, const Decimal low, const Decimal high){
	return low < var && var < high;
}

static inline void  swapCharPtr( char ** ptr1, char ** ptr2){
	char *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

#ifndef NETWORK_LOGGING
   #define NETWORK_LOGGING 0
#endif
#if(NETWORK_LOGGING != 2 && NETWORK_LOGGING != 1) 
    #undef NETWORK_LOGGING
    #define NETWORK_LOGGING 0
#endif
#define NETWORK_LOG_LEVEL_2_NUM(s,d) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s %d\n",(s), (d) );
#define NETWORK_LOG_LEVEL_2(s) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s\n", (s) );
#define NETWORK_LOG_LEVEL_1(s) if(NETWORK_LOGGING >= 1) fprintf(stderr, "%s\n", (s) );

int marker_controller(const struct http_request * request, char ** stringToReturn, int strLength){
	int status;
	int buffSize;
	int numParams;
	int page;
	long id; 
	char * buffer; 
	char tempBuf[40];
	char **convertSuccess;
	char * bufferRealloc;
	Decimal * latDegrees;
	Decimal * lonDegrees;
	Decimal * latOffset;
	Decimal * lonOffset;
	StrMap * sm;
	
	status = 500;
	convertSuccess = NULL;
	bufferRealloc = NULL;
	buffSize = 256;
	bzero(tempBuf, sizeof tempBuf);
	page = 1;

	/*Buffer up a good size that will probably not get filled (hopefully)*/
	buffer = malloc(buffSize);
	if(buffer == NULL){		
		goto mc_nomem;
	}
	buffer = memset(buffer,0,buffSize);

	latDegrees = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer);		
		goto mc_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}
	lonDegrees = malloc(sizeof(Decimal));
	if (lonDegrees == NULL) {
		free(buffer); free(latDegrees);				
		goto mc_nomem;
	} else {
		lonDegrees = memset(lonDegrees,0,sizeof(Decimal));
	}
	latOffset = malloc(sizeof(Decimal));
	if (latOffset == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees);		
		goto mc_nomem;
	} else {
		latOffset = memset(latOffset,0,sizeof(Decimal));
	}
	lonOffset = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset);		
		goto mc_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){		
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
		goto mc_nomem;
	}

	numParams = parseURL(request->url, strlen(request->url), sm);
	if(numParams > 0){
		/* Collect any parameters and convert them to the proper types */
		if (sm_exists(sm,"latdegrees")==1) {
			sm_get(sm,"latdegrees",tempBuf,sizeof tempBuf);
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				(*latDegrees) = createDecimalFromString(tempBuf);
			else{
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				status = 400;
				goto mc_badLatitude;
			}			
		} else{
			/* Sensible default for latdegrees is... to become NULL 
			 * so that we can handle using a different query without 
			 * relying on magic numbers of any kind. 
			*/
			free(latDegrees);
			latDegrees = NULL;
		}
		if (sm_exists(sm,"latoffset")==1) {
			sm_get(sm,"latoffset", tempBuf, sizeof tempBuf);
			/* Validate the numericness of the offset */
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				(*latOffset) = createDecimalFromString(tempBuf);
			else{
				status = 400;
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_badLatOffset;
			} 
		} else {
			(*latOffset) = createDecimalFromString(DEFAULT_OFFSET);
		}
		if (sm_exists(sm,"londegrees")==1) {
			sm_get(sm,"londegrees",tempBuf,sizeof tempBuf);
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				(*lonDegrees) = createDecimalFromString(tempBuf);
			else{
				status = 400;
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_badLongitude;
			}
		} else {
			free(lonDegrees);
			lonDegrees = NULL;
		}
		if (sm_exists(sm,"lonoffset")==1) {
			sm_get(sm,"lonoffset", tempBuf, sizeof tempBuf);
			convertSuccess = NULL;	
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				(*lonOffset) = createDecimalFromString(tempBuf);
			else{
				status = 400;
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				goto mc_badLonOffset;
			} 
		} else {
			(*lonOffset) = createDecimalFromString(DEFAULT_OFFSET);
		}

		if(sm_exists(sm, "page") == 1)
			if(sm_get(sm, "page", tempBuf, sizeof tempBuf) == 1){
				if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
					page = atoi(tempBuf);
				else
					page = -1;
				if(page <= 0){
					status = 400;		
					sm_delete(sm);
					free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);	
					goto mc_badpage;
				}

			}

	}else{
		/* We've got no parameters so free up the memory we don't need */\
		free(latDegrees);
		free(lonDegrees);
		free(latOffset);
		free(lonOffset);
		latDegrees = NULL;
		lonDegrees = NULL;
		latOffset  = NULL;
		lonOffset  = NULL;
	}
	/* Check Method and fly off the handle */
	switch(request->method){
		case GET:
			/* GET is the painful one for this controller:
			 * It takes _optional_ parameters, which means modifying our querying
			 * based on the parameters we have. 
			*/
			if((sm_exists(sm,"latoffset")==1) ^ (sm_exists(sm,"lonoffset")==1)){
				/*Err! if one is used, both must be used! */
				sm_delete(sm);
				free(buffer);
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				status = 400;
				goto mc_bothOffsets;
			}
			if(latDegrees != NULL)
				if( ! between(*latDegrees, -91.0, 91.0) ){
					sm_delete(sm);
					free(buffer);
					FREE_NON_NULL_DEGREES_AND_OFFSETS
					status = 422;
					goto mc_oobLatitude;		
				}

			if(lonDegrees != NULL)
				if( ! between(*lonDegrees, -181L, 181L) ){
					sm_delete(sm);
					free(buffer); 
					FREE_NON_NULL_DEGREES_AND_OFFSETS
					status = 422;
					goto mc_oobLongitude;			
				}
			if(sm_exists(sm,"id")!=1){
				/* Retrieve multiple markers */
				status = marker_get(&buffer,buffSize,latDegrees,lonDegrees,latOffset,lonOffset,page);
			}else{
				sm_get(sm, "id", tempBuf, sizeof tempBuf);
				id = atol(tempBuf);	
				status = marker_get_single(buffer, buffSize, id);
			}
			

			break;
		case POST:
			status = marker_post(buffer,buffSize,request);
			if(status == -1){
				/* Something went terribly wrong */
				sm_delete(sm);
				free(buffer); 
				status= 500;
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_nomem;
			}
			break;
		case PUT:
			if(sm_exists(sm,"id")!=1){
				status = 422;
				sm_delete(sm);
				free(buffer); 
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_missing_key;
			}
			sm_get(sm, "id", tempBuf, sizeof tempBuf);
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				id = atol(tempBuf);
			else{
				status = 422;
				sm_delete(sm);
				free(buffer);
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_nan_id;
			}
			status = marker_address(buffer,buffSize,id,request);
			break;
		case DELETE:
			if(sm_exists(sm,"id")!=1){
				status = 422;
				sm_delete(sm);
				free(buffer); 
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_missing_key;
			}
			sm_get(sm, "id", tempBuf, sizeof tempBuf);
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(tempBuf, "nan", 3) != 0)
				id = atol(tempBuf);
			else{
				status = 422;
				sm_delete(sm);
				free(buffer);
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_nan_id;
			}
			status = marker_delete(buffer, buffSize, id);
			if(status == -1){
				sm_delete(sm);
				free(buffer); 
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				goto mc_nomem;	
			}
			break;
		default:
			status = 501;	
			free(buffer); 
			FREE_NON_NULL_DEGREES_AND_OFFSETS
			sm_delete(sm);
			goto mc_unsupportedMethod;
	}

	if(strlen(buffer) > (unsigned)strLength){
		/* Can we get something bigger? 
		 * reuse some old variables for storing
		*/
		numParams = strlen(buffer);
		bufferRealloc = malloc(numParams*2);
		if(bufferRealloc == NULL){
			/* Out of memory... warn the server it's going to get
			 * a truncated response to the client in the logs. Not much
			 * we can do though.
			 */
			NETWORK_LOG_LEVEL_1("Not Enough memory to send valid response");
			NETWORK_LOG_LEVEL_2_NUM("Failed to allocate reallocation buffer for size",(int)numParams*2);
		}else{
			/* Swap the stringToReturn buffer and the realloced buffer */
			memset(bufferRealloc,0,numParams*2);

			swapCharPtr(stringToReturn,&bufferRealloc);
			free(bufferRealloc);
			strLength = numParams*2;
		}
	}
	swapCharPtr(stringToReturn, &buffer);
	free(buffer); 
	FREE_NON_NULL_DEGREES_AND_OFFSETS
	sm_delete(sm);	
	return status;

	ERR_LABEL_STRING_TO_RETURN(mc_nomem, NOMEM_ERROR)
	ERR_LABEL_STRING_TO_RETURN(mc_unsupportedMethod, BAD_METHOD_ERR)
	ERR_LABEL_STRING_TO_RETURN(mc_missing_key, MISSING_ID_KEY)
	ERR_LABEL_STRING_TO_RETURN(mc_badLonOffset, BAD_LON_OFFSET)
	ERR_LABEL_STRING_TO_RETURN(mc_badLatOffset, BAD_LAT_OFFSET)
	ERR_LABEL_STRING_TO_RETURN(mc_bothOffsets, BOTH_OFFSET_ERR)
	ERR_LABEL_STRING_TO_RETURN(mc_badpage, BAD_PAGE_ERR)
	ERR_LABEL_STRING_TO_RETURN(mc_nan_id, NAN_ID)
	ERR_LABEL_STRING_TO_RETURN(mc_badLatitude, NAN_LATITUDE)
	ERR_LABEL_STRING_TO_RETURN(mc_badLongitude, NAN_LONGITUDE)
	ERR_LABEL_STRING_TO_RETURN(mc_oobLatitude, OOB_LATITUDE)
	ERR_LABEL_STRING_TO_RETURN(mc_oobLongitude, OOB_LONGITUDE)

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
	struct gs_marker marker;

	gs_marker_ZeroStruct(&marker);

	affected = addressed =  0;

	/* Retrieve the put data first */
	if(request->contentLength > 0){
		charP = strstr(request->data, "addressed");
		if(charP == NULL){
			snprintf(buffer, buffSize,ERROR_STR_FORMAT,400,NO_ADDRESSED_KEY);
			return 400;
		} else {
			/* Key was found, skip the field name and find the value*/
			charP+=9;
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

	db_getMarkerById(id, &marker , conn);

	if(marker.id == GS_MARKER_INVALID_ID){
		mysql_close(conn);
		mysql_thread_end();
		snprintf(buffer, buffSize, "{\"status_code\" : 404,\"message\" : \"Could not find marker with given id\"}");
		return 404;
	}

	/* Mark addressed */	
	affected = db_addressMarker(id, addressed, conn);	

	mysql_close(conn);
	mysql_thread_end();

	if(affected > 0){
		snprintf(buffer, buffSize, "{\"status_code\" : 200,\"message\":\"successful update\"}");
	}else{
		snprintf(buffer,buffSize,"{\"status_code\" : 200,\"message\" : \"successful: no change\"}");
	}

	return 200;
}

int marker_post(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_marker marker;
	struct gs_comment assocComment;
	StrMap * sm;
	char keyBuffer[GS_COMMENT_MAX_LENGTH+1];
	char valBuffer[GS_COMMENT_MAX_LENGTH+1];
	Decimal longitude;
	Decimal latitude;
	char **convertSuccess;



	bzero(keyBuffer,sizeof keyBuffer);
	bzero(valBuffer,sizeof valBuffer);
	gs_marker_ZeroStruct(&marker);
	gs_comment_ZeroStruct(&assocComment);
	convertSuccess = NULL;

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		fprintf(stderr, "sm err\n");
		return -1;
	}

	/*Parse the JSON for the information we desire */
	parseJSON(request->data, request->contentLength, sm);
	
	/* Verify that the data is valid */
	if(	sm_exists(sm, "type") 		!=1 || 
		sm_exists(sm, "message") 	!=1 ||
		sm_exists(sm, "latdegrees") !=1 ||
		sm_exists(sm, "londegrees") !=1 ||
		sm_exists(sm, "addressed" ) !=1	){

		sm_delete(sm);
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,KEYS_MISSING);
		return 400;		
	}else{
		/* Extract and create the two structs */
		if(sm_get(sm,"type", valBuffer, sizeof valBuffer) == 1){
			/* Verify that it is a correct type */
			if(strncasecmp(valBuffer, CTYPE_1,COMMENTS_CTYPE_SIZE) != 0)
				if(strncasecmp(valBuffer, CTYPE_2,COMMENTS_CTYPE_SIZE) != 0)
					if(strncasecmp(valBuffer, CTYPE_3,COMMENTS_CTYPE_SIZE) != 0)
						if(strncasecmp(valBuffer, CTYPE_4, COMMENTS_CTYPE_SIZE) != 0){
						sm_delete(sm);
						snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,BAD_TYPE_ERR);
						return 422;
					}
		}
		/* _shared_campaign_id is a global inherited from green-serv.c */
		gs_comment_setScopeId(_shared_campaign_id, &assocComment);
		gs_marker_setScopeId(_shared_campaign_id, &marker);
		gs_comment_setCommentType(valBuffer,&assocComment);

		sm_get(sm,"message",valBuffer, sizeof valBuffer);
		if( strlen( valBuffer ) == 0 ){
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,EMPTY_COMMENT_MESSAGE);
			return 422;
		}
		gs_comment_setContent(valBuffer,&assocComment);

		sm_get(sm,"londegrees",valBuffer,sizeof valBuffer);
		if(! ( strtod(valBuffer,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(valBuffer, "nan", 3) != 0) ){
			sm_delete(sm);
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 400, NAN_LONGITUDE);
			return 400;
		}

		longitude = createDecimalFromString(valBuffer);
		if( ! between(longitude, -181L, 181L) ){
			sm_delete(sm);
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 422, OOB_LONGITUDE);
			return 422;
		}
		gs_marker_setLongitude(longitude, &marker);

		sm_get(sm,"latdegrees",valBuffer,sizeof valBuffer);
		if(! ( strtod(valBuffer,convertSuccess) != 0 && convertSuccess == NULL && strncasecmp(valBuffer, "nan", 3) != 0) ){
			sm_delete(sm);
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 400, NAN_LATITUDE);
			return 400;
		}
		latitude = createDecimalFromString( valBuffer);
		if( ! between(latitude, -91L, 91L) ){
			sm_delete(sm);
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 422, OOB_LATITUDE);
			return 422;
		}
		gs_marker_setLatitude(latitude, &marker);

		sm_get(sm,"addressed", valBuffer, sizeof valBuffer);
		if(strncasecmp(valBuffer,"true", sizeof valBuffer) == 0)
			gs_marker_setAddressed(ADDRESSED_TRUE,&marker);
		else if(strncasecmp(valBuffer,"false",sizeof valBuffer) == 0)
			gs_marker_setAddressed(ADDRESSED_FALSE,&marker);
		else{
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,NO_ADDRESSED_KEY);
			return 400;
		}
			

	
	}

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	db_start_transaction(conn);
	/* Insert comment first  because our mySQL trigger
	 * will then handle updating the comment's pin id to match the new pin
	 * that we'll submit after. If we didn't have this trigger we'd have
	 * to do things  a bit differently.
	*/
	db_insertComment(&assocComment,conn);
	if(assocComment.id == GS_COMMENT_INVALID_ID){
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,"Could not create pin because of invalid message or type");
		goto cleanup_on_err;
	}
	gs_marker_setCommentId(assocComment.id, &marker);
	db_insertMarker(&marker,conn);
	if(marker.id == GS_MARKER_INVALID_ID){
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,"The pin was unprocessable and could not be created");
		/* Clean up after ourselves */
		db_deleteComment(assocComment.id,conn);
		goto cleanup_on_err;
	}

	db_end_transaction(conn);

	mysql_close(conn);
	mysql_thread_end();
	sm_delete(sm);

	snprintf(buffer,buffSize,"{\"pin_id\":%ld, \"status_code\" : 200, \"message\" : \"Successful submit\" }", marker.id);

	return 200;

	cleanup_on_err:
		db_abort_transaction(conn);
		db_end_transaction(conn);
		mysql_close(conn);
		mysql_thread_end();
		sm_delete(sm);
		return -1;
}


int marker_get(char ** buffer,int buffSize,Decimal * latDegrees, Decimal * lonDegrees, Decimal * latOffset,Decimal * lonOffset,int page){
	MYSQL * conn;
	struct gs_comment * comments;
	struct gs_marker * markers; 
	int numMarkers;
	int nextPage;
	char nextStr[MAX_URL_LENGTH];
	char prevStr[MAX_URL_LENGTH];
	char * hybridBuffer;
	char json[512]; /*Some large enough number to hold all the info*/
	int i;
	
	char * tmpBuff;
	int resize;


	hybridBuffer = malloc(buffSize);
	if(hybridBuffer == NULL){
		return -1;
	}
	memset(hybridBuffer,0,buffSize);

	comments = malloc(MARKER_LIMIT * sizeof(struct gs_comment));
	if(comments == NULL){
		free(hybridBuffer);
		return -1; /* Return flag to send self to cc_nomem */
	}
	memset(comments,0,MARKER_LIMIT * sizeof(struct gs_comment));

	markers = malloc(MARKER_LIMIT * sizeof(struct gs_marker));
	if(markers == NULL){
		free(comments);
		free(hybridBuffer);
		return -1;
	}
	memset(markers,0,MARKER_LIMIT * sizeof(struct gs_marker));

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		free(hybridBuffer);
		free(comments);
		free(markers);
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}

	bzero(nextStr, sizeof nextStr);
	bzero(prevStr, sizeof prevStr);
	bzero(hybridBuffer,sizeof hybridBuffer);
	numMarkers = 0;

	if(latDegrees == NULL && lonDegrees == NULL){
		/* Easy, do a query for all the points regardless of location 
		 * Negative 1 on the page because we need to start the offset at 0
		*/
		numMarkers = db_getMarkerComments(page-1, _shared_campaign_id , markers, comments, conn);
	} else if ( lonDegrees == NULL && latDegrees != NULL) {
		/* Only caring about latdegrees */
		numMarkers = db_getMarkerCommentsLatitude(page-1, _shared_campaign_id, markers, comments, conn, latDegrees, latOffset);
	} else if ( lonDegrees != NULL && latDegrees == NULL ) {
		numMarkers = db_getMarkerCommentsLongitude(page-1, _shared_campaign_id, markers, comments, conn, lonDegrees, lonOffset);		
	} else if ( lonDegrees != NULL && latDegrees != NULL) {
		numMarkers = db_getMarkerCommentsFullFilter(page-1, _shared_campaign_id, markers, comments,  conn,latDegrees,latOffset, lonDegrees,  lonOffset);
	} else {
		/* Bad Request? not sure if it's possible to even hit this case */
		fprintf(stderr, "--%s\n", "Possible to hit here?");
	}

	mysql_close(conn);
	mysql_thread_end();

	if( numMarkers > MARKER_RETURNED ){
		nextPage = page+1;
		/*Need to tack on url parameters if present*/
		snprintf(nextStr,MAX_URL_LENGTH, "%spins?page=%d", BASE_API_URL, nextPage);
	} else {
		snprintf(nextStr,MAX_URL_LENGTH, "null");
	}

	if(page > 1)
		snprintf(prevStr,MAX_URL_LENGTH,"%spins?page=%d",BASE_API_URL,page-1);
	else
		snprintf(prevStr,MAX_URL_LENGTH,"null");

	/* Finally, create a hybrid json object and store it into the correct
	 * format and return the buffer to the calling function.
	*/
	resize = 1;
	for(i=0; i < min(numMarkers,MARKER_RETURNED); ++i){
		bzero(json,sizeof json);
		/* Custom hyrbid json call
		*/
		gs_markerCommentNToJSON(&markers[i], &comments[i] ,json, sizeof json);
		if(i==0)
			snprintf(hybridBuffer,buffSize,"%s",json);
		else{
			if((int)(strlen(hybridBuffer) + strlen(json)) > buffSize*resize){
				/* Resize both the hybrid buffer and the actual buffer. */
				resize = resize*2;
				tmpBuff = malloc(sizeof(char)*buffSize*resize);
				if(tmpBuff == NULL){
					NETWORK_LOG_LEVEL_1("WARN: Possible invalid JSON being sent to due re-allocation err");
					resize = resize/2; 
				}else{
					/* Successful sizing, so move the memory */
					strcpy(tmpBuff, hybridBuffer);
					swapCharPtr(&tmpBuff,&hybridBuffer);
					free(tmpBuff); /* free up the old memory */
				}
			}
			strncat(hybridBuffer,",",buffSize*resize);
			strncat(hybridBuffer,json,buffSize*resize);
		}			
	}

	/* If we've resized in the loop we need to resize the buffer */
	if(resize != 1){
		tmpBuff = malloc(sizeof(char)*buffSize*resize+256); /* 256 for the marker page str */
		if(tmpBuff == NULL){
			NETWORK_LOG_LEVEL_1("WARN: Possible invalid JSON being sent to due re-allocation err");
		}else{
			swapCharPtr(&tmpBuff,buffer);
			free(tmpBuff);
			buffSize = resize*buffSize+256;
		}
	}
	snprintf(*buffer,buffSize, MARKER_PAGE_STR, 200, hybridBuffer, min(numMarkers,MARKER_RETURNED), page-1, nextStr,prevStr);
	free(hybridBuffer);
	free(comments);
	free(markers);
	return 200;
}

/* /api/pins?id=<pin id> */
int marker_get_single(char * buffer,  int buffsize, long id){
	MYSQL *conn;
	struct gs_comment gsc;
	struct gs_marker gsm;
	char json[512];

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	db_getMarkerById(id, &gsm, conn);
	if(gsm.id == GS_MARKER_INVALID_ID){
		/* 404 */
		mysql_close(conn);
		mysql_thread_end();
		snprintf(buffer, buffsize, ERROR_STR_FORMAT, 404, "Could not find pin with given id");
		return 404;
	}else{
		db_getCommentById(gsm.commentId, &gsc, conn);
	}

	mysql_close(conn);
	mysql_thread_end();

	gs_markerCommentNToJSON(&gsm, &gsc ,json, sizeof json);

	snprintf(buffer,buffsize,"{\"status_code\" : 200,\"pin\" : %s}",json);
	return 200;
}