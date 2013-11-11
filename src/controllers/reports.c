#include "controllers/reports.h"

static inline int min(const int a, const int b){
	return a < b ? a : b;
}


int report_controller(const struct http_request * request, char * stringToReturn, int strLength){
	int status;
	int buffSize;
	int numParams;
	int page;
	char * buffer; 
	char tempBuf[100];
	char hash[65]; /* SHA256 or md5 */
	char since[20]; /* YYYY-mm-dd-HH:MM */
	char origin[100]; 
	StrMap * sm;
	unsigned int i;
	int valid;

	status = 503;
	buffSize = RESULTS_PER_PAGE*(sizeof(struct gs_report)*4+1)+(2*MAX_URL_LENGTH);
	bzero(tempBuf, sizeof tempBuf);
	page = 1;
	valid = 1;

	


	buffer = malloc(buffSize);
	if(buffer == NULL){
		status = 500;
		goto rc_nomem;
	}
	buffer = memset(buffer,0,buffSize);

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		free(buffer);
		goto rc_nomem;
	}
	numParams = parseURL(request->url, strlen(request->url), sm);
	if(numParams > 0){
		if(sm_exists(sm,"page") == 1){
			sm_get(sm, "page", tempBuf, sizeof tempBuf);
			page = atoi(tempBuf);
			if(page <= 0){
				status = 400;		
				sm_delete(sm);
				free(buffer); 
				goto rc_badpage;
			}
		}
		if(sm_exists(sm,"hash") == 1){
			sm_get(sm,"hash",hash, sizeof hash);
		}
		if(sm_exists(sm,"hash") && sm_exists(sm,"page")){
			free(buffer);
			sm_delete(sm);
			status = 422;
			goto rc_mutual_exclusion;
		}
		if(sm_exists(sm,"since") == 1){
			sm_get(sm,"since",since, sizeof since);
			for (i = 0; i < sizeof since; ++i){
				if(i==4 || i == 7 || i ==10 || i ==13){
					if(since[i] != '-')
						valid = 0;
				}else{
					if(!isdigit(since[i]))
						valid = 0;
				}
			}
			if(!valid){
				free(buffer);
				sm_delete(sm);
				goto rc_bad_date;
			}
		}
		if(sm_exists(sm,"origin") ==1 ){
			sm_get(sm,"origin",origin ,sizeof origin);
		}
	}
	sm_delete(sm);

	switch(request->method){
		case POST:	
			status = report_post(buffer, buffSize, request);
			break;
		case GET:
		case DELETE:
		case PUT:
		default:
			status = 501;	
			free(buffer); 			
			goto rc_unsupportedMethod;
	}

	
	snprintf(stringToReturn, strLength, "%s", buffer);
	free(buffer); 
	return status;

	rc_nomem:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, NOMEM_ERROR);
		return status;
	rc_badpage:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_PAGE_ERR);
		return status;
	rc_mutual_exclusion:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_PAGE_SINCE_ERR);
		return status;
	rc_bad_date:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_FORMAT_DATE_ERR);
		return status;
	rc_unsupportedMethod:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_METHOD_ERR);
		return status;

}

int report_delete(char * buffer, int buffSize, long id);

int report_post(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_report report;
	StrMap * sm;
	int i;
	int j;
	int strFlag;
	char keyBuffer[GS_REPORT_MAX_LENGTH+1];
	char valBuffer[GS_REPORT_MAX_LENGTH+1];
	char message[GS_REPORT_MAX_LENGTH+1];
	char stackTrace[GS_REPORT_MAX_LENGTH+1];
	char origin[SHA_LENGTH+1];
	char *auth;


	bzero(keyBuffer,sizeof keyBuffer);
	bzero(valBuffer,sizeof valBuffer);
	bzero(message, sizeof message);
	bzero(stackTrace, sizeof stackTrace);
	bzero(origin, sizeof origin);
	gs_report_ZeroStruct(&report);
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
	if(	sm_exists(sm, "stacktrace") !=1 || 
		sm_exists(sm, "message") 	!=1 ||
		sm_exists(sm, "origin") !=1 	){
		sm_delete(sm);
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,400,KEYS_MISSING);
		return 400;		
	}else{
		/* Extract and create the two structs */
		
		sm_get(sm,"message",valBuffer,sizeof valBuffer);
		if(strlen(valBuffer) == 0){
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,EMPTY_MESSAGE_ERR);
			return 422;			
		}
		snprintf(message, sizeof message,"%s", valBuffer);
		gs_report_setContent(message, &report);
		
		bzero(valBuffer,sizeof valBuffer);
		sm_get(sm,"stacktrace",valBuffer,sizeof valBuffer);
		if(strlen(valBuffer) == 0){
			sm_delete(sm);
			snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,EMPTY_TRACE_ERR);
			return 422;				
		}
		snprintf(stackTrace, sizeof stackTrace, "%s", valBuffer);
		/* This is where a setStackTrace func call goes once implemented */

		sm_get(sm,"origin", valBuffer, sizeof valBuffer);
		if(strlen(valBuffer) == 0){
			sm_delete(sm);
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 422, EMPTY_ORIGIN_ERR);
			return 422;
		}
		snprintf(origin, sizeof origin, "%s", valBuffer);
		gs_report_setOrigin(origin, &report);
	}

	gs_report_setScopeId(_shared_campaign_id, &report);
	/* Compute the authorization by combining the error message and stacktrace together */
	auth = strncat(message,stackTrace,sizeof message);
	gs_report_setAuthorize(auth, &report);

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	/* Insert comment first  because our mySQL trigger
	 * will then handle updating the comment's pin id to match the new pin
	 * that we'll submit after. If we didn't have this trigger we'd have
	 * to do things  a bit differently.
	*/
	db_insertReport(&report,conn);
	if(report.id == GS_REPORT_INVALID_ID){
		snprintf(buffer,buffSize,ERROR_STR_FORMAT,422,"Could not create report in database for some reason");
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

int report_get(char * buffer,int buffSize, char * hash, char * since, int page);