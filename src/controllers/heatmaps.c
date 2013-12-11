#include "controllers/heatmaps.h"

static inline int min(const int a, const int b){
	return a < b ? a : b;
}

#ifndef FREE_NON_NULL_DEGREES_AND_OFFSETS
	#define FREE_NON_NULL_DEGREES_AND_OFFSETS \
		if(latDegrees != NULL)\
			free(latDegrees);\
		if(lonDegrees != NULL)\
			free(lonDegrees); \
		if(latOffset != NULL)\
			free(latOffset);\
		if(lonOffset != NULL)\
			free(lonOffset);
#endif

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
		}else{
			free(latDegrees);
			latDegrees = NULL;
		}
		if( sm_exists(sm, "londegrees") == 1){
			sm_get(sm,"londegrees",tempBuf,sizeof tempBuf);
			(*lonDegrees) = createDecimalFromString(tempBuf);
		}else{
			free(lonDegrees);
			lonDegrees = NULL;
		}
		if( sm_exists(sm, "lonoffset") == 1){
			sm_get(sm,"lonoffset",tempBuf,sizeof tempBuf);
			(*lonOffset) = createDecimalFromString(tempBuf);
		} else {
			(*lonOffset) = createDecimalFromString(DEFAULT_OFFSET);
		}
		if( sm_exists(sm, "latoffset") == 1){
			sm_get(sm,"latoffset",tempBuf,sizeof tempBuf);
			(*latOffset) = createDecimalFromString(tempBuf);
		} else {
			(*latOffset) = createDecimalFromString(DEFAULT_OFFSET);
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
				free(buffer); 
				if(latDegrees != NULL)
					free(latDegrees); 
				if(lonDegrees != NULL)
					free(lonDegrees); 
				free(latOffset); free(lonOffset);	
				goto bad_precision;
			}
		}
		if( sm_exists(sm, "raw") == 1){
			sm_get(sm,"raw",tempBuf, sizeof tempBuf);
			if(strncasecmp(tempBuf,"true",4) == 0)
				raw = TRUE;
			else
				raw = FALSE;
		}
	}else{
		free(lonOffset); lonOffset  = NULL;	
		free(latOffset); latOffset  = NULL;
		free(lonDegrees); lonDegrees = NULL;
		free(latDegrees); latDegrees = NULL;

	}

	switch(request->method){
		case GET:
			/* Optional Parameters cause the database query to change
			*/
			if((sm_exists(sm,"latoffset")==1) ^ (sm_exists(sm,"lonoffset")==1)){
				/*Err! if one is used, both must be used! */
				sm_delete(sm);
				free(buffer);
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				
				status = 422;
				goto mh_bothOffsets;
			}
			if(latDegrees != NULL)
				/*Let the -90.1 slide by as ok...*/
				if(*latDegrees < -90L || *latDegrees > 90L){
					sm_delete(sm);
					free(buffer); 
					FREE_NON_NULL_DEGREES_AND_OFFSETS
					status = 422;
					goto mh_badlat;		
				}

			if(lonDegrees != NULL)
				if(*lonDegrees < -180L || *lonDegrees > 180L){
					sm_delete(sm);
					free(buffer); 
					FREE_NON_NULL_DEGREES_AND_OFFSETS
					
					status = 422;
					goto mh_badlon;			
				}
			status = heatmap_get(buffer, buffSize,page, latDegrees, latOffset, lonDegrees, lonOffset, precision, raw);
			if(status == -1){
				free(buffer); 
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				
				sm_delete(sm);
				goto mh_nomem;
			}
			break;
		case PUT:
			status = heatmap_put(buffer,buffSize,request);
			if(status == -1){
				free(buffer); 
				FREE_NON_NULL_DEGREES_AND_OFFSETS
				
				sm_delete(sm);	
				goto mh_nomem;
			}
			break;
		case DELETE:
		case POST:	
		default:
			status = 501;	
			free(buffer); 
			FREE_NON_NULL_DEGREES_AND_OFFSETS
			
			sm_delete(sm);
			goto mh_unsupportedMethod;
	}

	
	snprintf(stringToReturn, strLength, "%s", buffer);
	free(buffer); 
	FREE_NON_NULL_DEGREES_AND_OFFSETS
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
	MYSQL * conn;
	struct gs_heatmap * heatmaps;
	int numHeatmaps;
	int nextPage;
	long max;
	char nextStr[MAX_URL_LENGTH];
	char prevStr[MAX_URL_LENGTH];
	char tempBuf[buffSize];
	Decimal lowerLat;
	Decimal upperLat;
	Decimal lowerLon;
	Decimal upperLon;

	char json[512]; /*Some large enough number to hold all the info*/
	int i;
	heatmaps = malloc(HEATMAP_RESULTS_PER_PAGE * sizeof(struct gs_heatmap));
	if(heatmaps == NULL){
		return -1; /* Return flag to send self nomem */
	}
	memset(heatmaps,0,HEATMAP_RESULTS_PER_PAGE * sizeof(struct gs_heatmap));
	lowerLon = createDecimalFromString("-181");
	lowerLat = createDecimalFromString("-91");
	upperLon = createDecimalFromString("181");
	upperLat = createDecimalFromString("91");

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		free(heatmaps);
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}

	bzero(nextStr, sizeof nextStr);
	bzero(prevStr, sizeof prevStr);
	bzero(tempBuf,sizeof tempBuf);
	numHeatmaps = 0;

	if(latDegrees == NULL && lonDegrees == NULL){
		/* Easy, do a query for all the points regardless of location 
		/ * Negative 1 on the page because we need to start the offset at 0
		*/
	} else if ( lonDegrees == NULL && latDegrees != NULL) {
		/* Only caring about latdegrees */
		add_decimals(latDegrees,latOffset,&upperLat);
		subtract_decimals(latDegrees,latOffset,&lowerLat);
	} else if ( lonDegrees != NULL && latDegrees == NULL ) {
		add_decimals(lonDegrees,lonOffset,&upperLon);
		subtract_decimals(lonDegrees,lonOffset,&lowerLon);
	} else if ( lonDegrees != NULL && latDegrees != NULL) {
		add_decimals(latDegrees,latOffset,&upperLat);
		subtract_decimals(latDegrees,latOffset,&lowerLat);
		add_decimals(lonDegrees,lonOffset,&upperLon);
		subtract_decimals(lonDegrees,lonOffset,&lowerLon);
	} else {
		/* Bad Request? not sure if it's possible to even hit this case */
		fprintf(stderr, "--%s\n", "Possible to hit here?");
	}
	/* Negative 1 on the page because we need to start the offset at 0	*/
	numHeatmaps = db_getHeatmap(page-1, _shared_campaign_id, precision, &max, lowerLat, upperLat, lowerLon, upperLon, heatmaps, conn);

	if( numHeatmaps > HEATMAP_RESULTS_RETURNED ){
		nextPage = page+1;
		/*Need to tack on url parameters if present*/
		snprintf(nextStr,MAX_URL_LENGTH, "%sheatmap?page=%d&raw=%s", BASE_API_URL, nextPage, raw == TRUE ? "true" : "false");
	} else {
		snprintf(nextStr,MAX_URL_LENGTH, "null");
	}

	if(page > 1)
		snprintf(prevStr,MAX_URL_LENGTH,"%sheatmap?page=%d&raw=%s",BASE_API_URL,page-1, raw == TRUE ? "true" : "false");
	else
		snprintf(prevStr,MAX_URL_LENGTH,"null");

	for(i=0; i < min(numHeatmaps,HEATMAP_RESULTS_RETURNED); ++i){
		bzero(json,sizeof json);
		if(!raw)
			heatmaps[i].intensity = (heatmaps[i].intensity/(double)max)*100;
		gs_heatmapNToJSON(heatmaps[i], json, sizeof json);
		if(i==0)
			snprintf(tempBuf,buffSize,"%s",json);
		else{
			strncat(tempBuf,",",buffSize);
			strncat(tempBuf,json,buffSize);
		}			
	}
	snprintf(buffer,buffSize, HEATMAP_PAGE_STR, 200, tempBuf, min(numHeatmaps,HEATMAP_RESULTS_RETURNED), page, nextStr,prevStr);
	
	free(heatmaps);
	mysql_close(conn);
	mysql_thread_end();

	return 200;
}

int heatmap_put(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_heatmap * heatmap;
	StrMap * sm;
	int i, j, strFlag;
	int numberHeatmapsSent;
	long intensity;
	char keyBuffer[GS_COMMENT_MAX_LENGTH+1];
	char valBuffer[GS_COMMENT_MAX_LENGTH+1];
	Decimal longitude;
	Decimal latitude;
	struct mNode * lhead;
	struct mNode * ltail;

	lhead = NULL;
	ltail  =NULL;
	bzero(keyBuffer,sizeof keyBuffer);
	bzero(valBuffer,sizeof valBuffer);
	
	strFlag = 0;

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		fprintf(stderr, "sm err\n");
		return -1;
	}

	numberHeatmapsSent = 0;
	/*Parse the JSON for the information we desire no using parseJSON here yet until refactor becuase of goto logic*/
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
			if(strlen(keyBuffer) > 0 && strlen(valBuffer) > 0){
				if(sm_put(sm, keyBuffer, valBuffer) == 0)
                	fprintf(stderr, "Failed to copy parameters into hash table while parsing url\n");
                else
                	numberHeatmapsSent++; /* Increase for each key val pair we find, then divide by 3 later */
            }
            /* Check if we have a full and valid point yet */
            if(sm_exists(sm, "secondsworked") == 1 && sm_exists(sm,"latdegrees") == 1 && sm_exists(sm,"londegrees") ==1 ){
            	heatmap = malloc(sizeof (struct gs_heatmap));
            	if(heatmap == NULL){
            		sm_delete(sm);
            		if(lhead != NULL)
            			destroy_list(lhead);
            		return -1;
            	}
            	gs_heatmap_ZeroStruct(heatmap);

            	/* Verify that the data is valid */
				if(	sm_exists(sm, "secondsworked") 	!=1 || 
					sm_exists(sm, "latdegrees") 	!=1 ||
					sm_exists(sm, "londegrees") 	!=1 ){
					
					sm_delete(sm);
					if(lhead != NULL)
            			destroy_list(lhead);
					snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,KEYS_MISSING);
					return 400;		
				}else{		
					/* _shared_campaign_id is a global inherited from green-serv.c */
					gs_heatmap_setScopeId(_shared_campaign_id, heatmap);		

					sm_get(sm,"londegrees",valBuffer,sizeof valBuffer);
					longitude = createDecimalFromString(valBuffer);
					gs_heatmap_setLongitude(longitude, heatmap);

					sm_get(sm,"latdegrees",valBuffer,sizeof valBuffer);
					latitude = createDecimalFromString( valBuffer);
					gs_heatmap_setLatitude(latitude, heatmap);

					/* Check latitude and longitude ranges */
					if(!(-90L <= latitude && latitude <= 90L )){
						sm_delete(sm);
						if(lhead != NULL)
            				destroy_list(lhead);
						snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,LATITUDE_OUT_OF_RANGE_ERR);
						return 400;
					}

					if(!(-180L <= longitude && longitude <= 180L)){
						sm_delete(sm);
						if(lhead != NULL)
            				destroy_list(lhead);
						snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,LONGITUDE_OUT_OF_RANGE_ERR);
						return 400;
					}

					sm_get(sm,"secondsworked", valBuffer, sizeof valBuffer);
					intensity = strtol(valBuffer,NULL,10);
					if(intensity > 0L)
						gs_heatmap_setIntensity(intensity, heatmap);
					else{
						if(intensity <= 0 ){
							sm_delete(sm);
							snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,BAD_INTENSITY_ERR);
							if(lhead != NULL)
            					destroy_list(lhead);
							return 400;		
					}else{
							sm_delete(sm);
							snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,BAD_INTENSITY_NEG_ERR);
							if(lhead != NULL)
            					destroy_list(lhead);
							return 422;	
						}	
					}
				}

				if(lhead == NULL)
					ltail = lhead = create_node(lhead, heatmap );
				else
					ltail = create_node(ltail, heatmap);

				sm_delete(sm);
            	sm = sm_new(HASH_TABLE_CAPACITY);
				if(sm == NULL){
					fprintf(stderr, "sm err\n");
					if(lhead != NULL)
            			destroy_list(lhead);
					return -1;
				}
            }
		}
		strFlag = 0;
	}

	sm_delete(sm);

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		destroy_list(lhead);
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	db_start_transaction(conn);
	/* Iterate over list */
	for(i=0, ltail = lhead; ltail != NULL; ltail = ltail->next,i++ ) {

		db_insertHeatmap( (struct gs_heatmap* )(ltail->data), conn);
		if( ((struct gs_heatmap*) ltail->data )->id == GS_HEATMAP_INVALID_ID){
			fprintf(stderr, "%s\n", "Unknown error occured, could not insert heatmap into database.");
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,"Could not create heatmap. An Unknown Request Error has occured");
			destroy_list(lhead);
			mysql_close(conn);
			mysql_thread_end();	
			return 422;
		}

	}

	if(i != (numberHeatmapsSent/3) || i == 0 ){
		db_abort_transaction(conn);
		db_end_transaction(conn);
		mysql_close(conn);
		mysql_thread_end();
		destroy_list(lhead);		
		if(i == 0){
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 422, "Must send data to be processed");
			return 422;
		}else{
			snprintf(buffer,buffSize,ERROR_STR_FORMAT, 400, KEYS_MISSING);
			return 400;	
		}		
	}
	db_end_transaction(conn); /* Still call end to reset autocommit to true */

	mysql_close(conn);
	mysql_thread_end();
	destroy_list(lhead);

	snprintf(buffer,buffSize,"{ \"status_code\" : 200, \"message\" : \"Successful submit\" }");

	return 200;		
}

#undef FREE_NON_NULL_DEGREES_AND_OFFSETS