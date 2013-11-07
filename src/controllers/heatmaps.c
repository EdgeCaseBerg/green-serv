#include "controllers/heatmaps.h"

int heatmap_controller(const struct http_request * request, char * stringToReturn, int strLength){
	int status;
	char * buffer; 
	int buffSize;
	int page;
	int raw;
	int numParams;
	int precision;
	char **convertSuccess;
	char tempBuf[40];
	StrMap * sm;
	Decimal * latDegrees;
	Decimal * lonDegrees;
	Decimal * latOffset;
	Decimal * lonOffset;


	status = 503;
	buffSize = HEATMAP_RESULTS_PER_PAGE*(sizeof(struct gs_heatmap)*4+1)+(2*MAX_URL_LENGTH);
	bzero(tempBuf, sizeof tempBuf);
	page = 1;
	precision = 8;
	convertSuccess = NULL;
	raw = FALSE;

	buffer = malloc(buffSize);
	if(buffer == NULL){
		status = 500;
		goto mh_nomem;
	}
	buffer = memset(buffer,0,buffSize);

	latDegrees = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer);
		status = 500;
		goto mh_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}
	lonDegrees = malloc(sizeof(Decimal));
	if (lonDegrees == NULL) {
		free(buffer); free(latDegrees);		
		status = 500;
		goto mh_nomem;
	} else {
		lonDegrees = memset(lonDegrees,0,sizeof(Decimal));
	}
	latOffset = malloc(sizeof(Decimal));
	if (latOffset == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees);
		status = 500;
		goto mh_nomem;
	} else {
		latOffset = memset(latOffset,0,sizeof(Decimal));
	}
	lonOffset = malloc(sizeof(Decimal));
	if (latDegrees == NULL) {
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset);
		status = 500;
		goto mh_nomem;
	} else {
		latDegrees = memset(latDegrees,0,sizeof(Decimal));
	}

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonDegrees);
		goto mh_nomem;
	}
	numParams = parseURL(request->url, strlen(request->url), sm);
	if(numParams > 0){
		/* Parse for optional parameters of page,latdegrees,londegrees,latoffsets,lonoffsets,precision, and raw*/
		if( sm_exists(sm, "page") ==1){
			sm_get(sm,"page",tempBuf,sizeof tempBuf);
			page = atoi(tempBuf);
			if(page <= 0){
				status = 400;		
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);	
				goto mh_badpage;
			}
		}
		if( sm_exists(sm,"latdegrees") == 1){
			sm_get(sm,"latdegrees",tempBuf,sizeof tempBuf);
			(*latDegrees) = createDecimalFromString(tempBuf);
		}
		if( sm_exists(sm, "londegrees") == 1){
			sm_get(sm,"londegrees",tempBuf,sizeof tempBuf);
			(*lonDegrees) = createDecimalFromString(tempBuf);
		}
		if( sm_exists(sm, "lonoffset") == 1){
			sm_get(sm,"lonoffset",tempBuf,sizeof tempBuf);
			(*lonOffset) = createDecimalFromString(tempBuf);
		}
		if( sm_exists(sm, "latoffset") == 1){
			sm_get(sm,"latoffset",tempBuf,sizeof tempBuf);
			(*latOffset) = createDecimalFromString(tempBuf);
		}
		if( sm_exists(sm, "precision") == 1){
			sm_get(sm,"precision",tempBuf, sizeof tempBuf);
			if(strtod(tempBuf,convertSuccess) != 0 && convertSuccess == NULL){
				precision = strtod(tempBuf,convertSuccess);
				if(precision < 0)
					goto hop;
			}else{
				hop:
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);	
				goto bad_precision;
			}
		}
		if( sm_exists(sm, "raw") == 1){
			sm_get(sm,"raw",tempBuf, sizeof tempBuf);
			if(strncasecmp(tempBuf,"true",4) == 0)
				raw = TRUE;
		}
	}

	switch(request->method){
		case GET:
			/* Optional Parameters cause the database query to change
			*/
			if((sm_exists(sm,"latoffset")==1) ^ (sm_exists(sm,"lonoffset")==1)){
				/*Err! if one is used, both must be used! */
				sm_delete(sm);
				free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
				status = 422;
				goto mh_bothOffsets;
			}
			if(latDegrees != NULL)
				/*Let the -90.1 slide by as ok...*/
				if(*latDegrees < -90L || *latDegrees > 90L){
					sm_delete(sm);
					free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
					status = 422;
					goto mh_badlat;		
				}

			if(lonDegrees != NULL)
				if(*lonDegrees < -180L || *lonDegrees > 180L){
					sm_delete(sm);
					free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
					status = 422;
					goto mh_badlon;			
				}
			status = heatmap_get(buffer, buffSize,page, latDegrees, latOffset, lonDegrees, lonOffset, precision, raw);
			break;
		case PUT:
			status = heatmap_put(buffer,buffSize,request);
			break;
		case DELETE:
		case POST:	
		default:
			status = 501;	
			free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);
			sm_delete(sm);
			goto mh_unsupportedMethod;
	}

	fprintf(stderr, "Processing by heatmap: Working with %p %s %d", (void*)request, stringToReturn, strLength);

	
	snprintf(stringToReturn, strLength, "%s", buffer);
	free(buffer); free(latDegrees); free(lonDegrees); free(latOffset); free(lonOffset);	
	sm_delete(sm);
	return status;

	mh_unsupportedMethod:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_METHOD_ERR);
		return status;

	mh_nomem:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, NOMEM_ERROR);
		return status;

	mh_badpage:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_PAGE_ERR);
		return status;		

	mh_badlat:
		snprintf(stringToReturn,strLength,ERROR_STR_FORMAT,status,LATITUDE_OUT_OF_RANGE_ERR);
		return status;

	mh_badlon:
		snprintf(stringToReturn,strLength,ERROR_STR_FORMAT,status,LONGITUDE_OUT_OF_RANGE_ERR);
		return status;

	mh_bothOffsets:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BOTH_OFFSET_ERR);
		return status;

	bad_precision:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, PRECISION_ERR);
		return status;

}

int heatmap_get(char * buffer, int buffSize,int page, Decimal * latDegrees, Decimal * latOffset, Decimal * lonDegrees, Decimal * lonOffset, int precision, int raw){
	fprintf(stderr, "%s %d %d "DecimalFormat DecimalFormat DecimalFormat DecimalFormat "%d %d\n", buffer, buffSize,page, *latDegrees, *latOffset,*lonDegrees,*lonOffset, precision,raw);
	return 503;
}

int heatmap_put(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_heatmap heatmap;
	StrMap * sm;
	int i;
	int j;
	int strFlag;
	long intensity;
	char keyBuffer[GS_COMMENT_MAX_LENGTH+1];
	char valBuffer[GS_COMMENT_MAX_LENGTH+1];
	Decimal longitude;
	Decimal latitude;


	bzero(keyBuffer,sizeof keyBuffer);
	bzero(valBuffer,sizeof valBuffer);
	gs_heatmap_ZeroStruct(&heatmap);
	strFlag = 0;

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		fprintf(stderr, "sm err\n");
		return -1;
	}

	/*Parse the JSON for the information we desire */
	for(i=0; i < request->contentLength && request->data[i] != '\0'; ++i){
		/*We're at the start of a string*/
		if(request->data[i] == '"'){
			/*Go until we hit the closing qoute*/
			i++;
			for(j=0; i < request->contentLength && request->data[i] != '\0' && request->data[i] != '"' && (unsigned int)j < sizeof keyBuffer; ++j,++i){
				keyBuffer[j] = (int)request->data[i] > 64 && request->data[i] < 91 ? request->data[i] + 32 : request->data[i];
			}
			keyBuffer[j] = '\0';
			/*find the beginning of the value
			 *which is either a " or a number. So skip spaces and commas
			*/
			for(i++; i < request->contentLength && request->data[i] != '\0' && (request->data[i] == ',' || request->data[i] == ' ' || request->data[i] == ':' || request->data[i] == '\n'); ++i)
				;
			/*Skip any opening qoute */
			if(request->data[i] != '\0' && request->data[i] == '"'){
				i++;
				strFlag = 1;
			}
			for(j=0; i < request->contentLength && request->data[i] != '\0'; ++j,++i){
				if(strFlag == 0){
					if(request->data[i] == ' ' || request->data[i] == '\n')
						break; /*break out if num data*/
				}else{
					if(request->data[i] == '"' && request->data[i-1] != '\\')
						break;
				}
				valBuffer[j] = request->data[i];
			}
			valBuffer[j] = '\0';
			/* Skip any closing paren. */
			if(request->data[i] == '"')
				i++;
			if(strlen(keyBuffer) > 0 && strlen(valBuffer) > 0)
				if(sm_put(sm, keyBuffer, valBuffer) == 0)
                	fprintf(stderr, "Failed to copy parameters into hash table while parsing url\n");
		}
		strFlag = 0;
	}

	/* Verify that the data is valid */
	if(	sm_exists(sm, "secondsworked") 	!=1 || 
		sm_exists(sm, "latdegrees") 	!=1 ||
		sm_exists(sm, "londegrees") 	!=1 ){
		sm_delete(sm);
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,KEYS_MISSING);
		return 400;		
	}else{		
		/* _shared_campaign_id is a global inherited from green-serv.c */
		gs_heatmap_setScopeId(_shared_campaign_id, &heatmap);		

		sm_get(sm,"londegrees",valBuffer,sizeof valBuffer);
		longitude = createDecimalFromString(valBuffer);
		gs_heatmap_setLongitude(longitude, &heatmap);

		sm_get(sm,"latdegrees",valBuffer,sizeof valBuffer);
		latitude = createDecimalFromString( valBuffer);
		gs_heatmap_setLatitude(latitude, &heatmap);

		/* Check latitude and longitude ranges */
		if(!(-90L <= latitude && latitude <= 90L )){
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,LATITUDE_OUT_OF_RANGE_ERR);
			return 400;
		}

		if(!(-180L <= longitude && longitude <= 180L)){
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,LONGITUDE_OUT_OF_RANGE_ERR);
			return 400;
		}

		sm_get(sm,"secondsworked", valBuffer, sizeof valBuffer);
		fprintf(stderr, "valbuff:%s\n", valBuffer);
		intensity = strtol(valBuffer,NULL,10);
		if(intensity > 0L)
			gs_heatmap_setIntensity(intensity, &heatmap);
		else{
			if(intensity <= 0 ){
				sm_delete(sm);
				snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,BAD_INTENSITY_ERR);
				return 400;		
			}else{
				sm_delete(sm);
				snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,BAD_INTENSITY_NEG_ERR);
				return 422;	
			}	
		}

	
	}

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	db_insertHeatmap(&heatmap,conn);
	if(heatmap.id == GS_HEATMAP_INVALID_ID){
		fprintf(stderr, "%s\n", "Unknown error occured, could not insert heatmap into database.");
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,"Could not create heatmap. An Unknown Request Error has occured");
		goto cleanup_on_err;
	}

	mysql_close(conn);
	mysql_thread_end();
	sm_delete(sm);

	snprintf(buffer,buffSize,"{ \"status_code\" : 200, \"message\" : \"Successful submit\" }");

	return 200;

	cleanup_on_err:
		mysql_close(conn);
		mysql_thread_end();
		sm_delete(sm);
		return -1;
}
