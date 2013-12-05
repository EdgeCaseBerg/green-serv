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
	char * defaultSince = "2000-01-01 00:00";
	char origin[100]; 
	StrMap * sm;
	unsigned int i;
	int valid;

	status = 503;
	buffSize = RESULTS_PER_PAGE*(sizeof(struct gs_report)*4+1)+(2*MAX_URL_LENGTH);
	bzero(tempBuf, sizeof tempBuf);
	bzero(origin,sizeof origin);
	bzero(hash, sizeof hash);
	bzero(since, sizeof since);
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
			for (i = 0; since[i] != '\0' && i < sizeof since; ++i){
				fprintf(stderr, "%d %d\n", i, valid);
				if(i==4 || i == 7 || i ==10 || i ==13){
					if(since[i] != '-')
						valid = 0;
					if(!valid && i == 10)
						if(since[i] == '+'){ /* space url encoded */
							valid = 1;
							since[i] = ' ';
						}
					if(!valid && i ==13)
						if(since[i] == ':')
							valid = 1;
				}else{
					if(!isdigit(since[i]))
						valid = 0;
				}
			}
			if(!valid){
				fprintf(stderr, "--:%s\n", since );
				free(buffer);
				sm_delete(sm);
				goto rc_bad_date;
			}
		}else{
			for(i=0; i < sizeof defaultSince; ++i){
				since[i] = defaultSince[i];
			}
			since[i] = '\0';
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
			status = report_get(buffer, buffSize, hash, since, page);
			break;
		case DELETE:
			status = report_delete( buffer, buffSize, origin, hash);
			break;
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

int report_delete(char * buffer, int buffSize, char * origin, char * hash){
	MYSQL *conn;
	struct gs_report report;
	char orig[SHA_LENGTH+1]; 

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	bzero(orig, sizeof orig);
	sha256(origin,orig);

	db_getReportByAuth(hash, &report, conn);

	if(report.id == GS_REPORT_INVALID_ID){
		/* Couldn't find.  */
		snprintf(buffer, buffSize, ERROR_STR_FORMAT, 404, REPORT_NOT_FOUND);
		mysql_close(conn);
		mysql_thread_end();
		return 404;
	}

	/* Check the origin's to determine permissions to delete */
	if(strncmp(report.origin, orig, SHA_LENGTH) != 0){
		snprintf(buffer, buffSize, ERROR_STR_FORMAT, 403, ORIGIN_NOT_ALLOWED);
		mysql_close(conn);
		mysql_thread_end();
		return 403;
	}

	if( db_deleteReport(&report, conn) > 0 ){
		snprintf(buffer, buffSize, "%s","");
		mysql_close(conn);
		mysql_thread_end();
		return 204;
	}else{
		snprintf(buffer, buffSize, "{\"status_code\" : 404 ,\"message\" : \"Successful Deletion\"}");
		mysql_close(conn);
		mysql_thread_end();
		return 404;		
	}
}

int report_post(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_report report;
	StrMap * sm;
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
	

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		fprintf(stderr, "sm err\n");
		return -1;
	}

	/*Parse the JSON for the information we desire */
	parseJSON(request->data,request->contentLength, sm);

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
		gs_report_setStackTrace(stackTrace, &report);

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

int report_get(char * buffer,int buffSize, char * hash, char * since, int page){
	MYSQL *conn;
	struct gs_report report;
	struct gs_report * reports;
	int reportBuffSize;
	int nextPage;
	char nextStr[MAX_URL_LENGTH];
	char prevStr[MAX_URL_LENGTH];
	char reports_buffer[buffSize];
	long numReports;
	char json[512];
	int i;
	
	nextPage = 1;
	numReports = 0;
	reportBuffSize = RESULTS_PER_PAGE* ( sizeof (struct gs_report));

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	bzero(json, sizeof json);


	if(strlen(hash) != 0){
		db_getReportByAuth(hash, &report, conn);

		if(report.id == GS_REPORT_INVALID_ID){
			/* Couldn't find.  */
			snprintf(buffer, buffSize, ERROR_STR_FORMAT, 404, REPORT_NOT_FOUND);
			mysql_close(conn);
			mysql_thread_end();
			return 404;
		}
		gs_reportNToJSON(report, json, sizeof json);
		snprintf(buffer, buffSize, "{\"status_code\" : 200,\"report\" : %s}",json);
	}else{
		/* Paginated GET */
		reports = malloc(reportBuffSize);
		if(reports == NULL){
			mysql_close(conn);
			mysql_thread_end();
			return -1;
		}
		memset(reports,0,reportBuffSize);
		bzero(nextStr, sizeof nextStr);
		bzero(prevStr, sizeof nextStr);

		numReports = db_getReports(page-1, since, _shared_campaign_id, reports, conn);

		if( numReports > RESULTS_RETURNED ){
			nextPage = page+1;
			snprintf(nextStr,MAX_URL_LENGTH, "%sdebug?page=%d&since=%s", BASE_API_URL, nextPage, since );
		} else {
			snprintf(nextStr,MAX_URL_LENGTH, "null");
		}

		if(page > 1)
			snprintf(prevStr,MAX_URL_LENGTH,"%sdebug?page=%d&since=%s",BASE_API_URL,page-1,since);
		else
			snprintf(prevStr,MAX_URL_LENGTH,"null");

		bzero(reports_buffer,sizeof reports_buffer);
		for(i=0; i < min(numReports,RESULTS_RETURNED); ++i){
			bzero(json,sizeof json);
			gs_reportNToJSON(reports[i] ,json, sizeof json);
			if(i==0)
				snprintf(reports_buffer,buffSize,"%s",json);
			else{
				strncat(reports_buffer,",",buffSize);
				strncat(reports_buffer,json,buffSize);
			}			
		}

		snprintf(buffer,buffSize, REPORT_PAGE_STR, 200, reports_buffer, min(numReports,RESULTS_RETURNED), page, nextStr,prevStr);
		free(reports);
	}

	mysql_close(conn);
	mysql_thread_end();
	return 200;
}