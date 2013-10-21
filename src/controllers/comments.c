#include "controllers/comments.h"

static inline int min(const int a, const int b){
	return a < b ? a : b;
}

/*Internal Routing for the comment controller.
 *Responsible to delegate to GET,POST,PUT,DELETE methods here.
 *Will return whatever the result of the request is into the 
 *stringToReturn. Will NOT overflow the buffer. If not enough space
 *is given controller may return an invalid response.
 *Returns the status code of the response.
*/
int comment_controller(const struct http_request * request, char * stringToReturn, int strLength){
	char cType[COMMENTS_CTYPE_SIZE];
	char buffer[(RESULTS_PER_PAGE * sizeof(struct gs_comment))*4+1+(2*MAX_URL_LENGTH)];
	char tempBuf[20];
	int page;
	int status;
	int numParams;
	long id;
	StrMap * sm;

	page=1;
	status = 503;
	bzero(cType, sizeof cType);
	bzero(buffer, sizeof buffer);
	bzero(tempBuf, sizeof tempBuf);

	

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		goto cc_nomem;
	}

	/* Parse the URL */
	numParams = parseURL(request->url, strlen(request->url), sm);
	if(numParams > 0){
		if(sm_exists(sm, "page") == 1)
			if(sm_get(sm, "page", tempBuf, sizeof tempBuf) == 1){
				page = atoi(tempBuf);
				if(page <= 0){
					/* Err */
					status = 400;			
					sm_delete(sm);
					goto cc_badpage;
				}

			}
		if(sm_exists(sm, "type") ==1){
			if(sm_get(sm,"type", cType, sizeof cType) == 1){
				/* Verify that it is a correct type */
				if(strncasecmp(cType, CTYPE_1,COMMENTS_CTYPE_SIZE) != 0)
					if(strncasecmp(cType, CTYPE_2,COMMENTS_CTYPE_SIZE) != 0)
						if(strncasecmp(cType, CTYPE_3,COMMENTS_CTYPE_SIZE) != 0){
							status = 400;					
							sm_delete(sm);
							goto cc_badtype;
						}

			}
		}else{
			/*No Type, set flag*/
			cType[0] = '\0';
		}
	}
	
	/* For the client we start numbering from 1, for interal use we need to use
	 * page-1 because the page is part of the calculation of the limit term in
	 * the query. So to get the first page it needs to be 0.
	*/
	page -=1;

	switch(request->method){
		case GET:
			if(cType[0] == '\0' ){
				if( comments_get(buffer, sizeof buffer ,page,NULL) == -1 ){
					sm_delete(sm);
					goto cc_nomem;
				}
			} else {
				if( comments_get(buffer, sizeof buffer ,page,cType) == -1 ){
					sm_delete(sm);
					goto cc_nomem;
				}
			}
			/* Process results of comments_get */
			snprintf(stringToReturn,strLength,"%s",buffer);
			status = 200;
			break;
		case POST:
			status = comment_post(buffer,sizeof buffer,request);
			if( status == -1 ){
				sm_delete(sm);
				goto cc_nomem;
			} else if(status == 400) {
				sm_delete(sm);
				goto cc_badtype;
			} else if(status == -400) {
				sm_delete(sm);
				goto cc_missing;
			}
			snprintf(stringToReturn,strLength,"%s",buffer);
			break;
		case DELETE:
			if(sm_exists(sm,"id")!=1){
				status = 400;
				sm_delete(sm);
				goto cc_missing_key;
			}
			sm_get(sm, "id", buffer, sizeof buffer);
			id = atol(buffer);
			status = comment_delete(buffer, sizeof buffer, id);
			snprintf(stringToReturn,strLength,"%s",buffer);
			break;
		default:
			/*Invalid Method Err*/
			status = 501;	
			sm_delete(sm);
			goto cc_unsupportedMethod;
			break;
	}
	sm_delete(sm);

	return status;

	cc_nomem: /*Comment Controller Memory Allocation fail */
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, 500, NOMEM_ERROR);
		return status;

	cc_badpage:/*Comment Controller Bad Page Request */
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_PAGE_ERR);
		return status;		

	cc_badtype:/*Comment Controller Bad type request */
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_TYPE_ERR);
		return status;	

	cc_unsupportedMethod:/*Comment Controller Bad method*/			
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, BAD_METHOD_ERR);
		return status;			

	cc_missing:/* Comment controller missing required keys */
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, MISSING_KEY_ERR);
		return status;					
	
	cc_missing_key:
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, MISSING_ID_KEY);
		return status;							

}

/*Accepts NULL for cType if no filter
 *page will default to 1 if negative values passed in or 0
 *
*/
int comments_get(char * buffer, int buffSize,int page,char * cType){
	struct gs_comment * commentPage;
	int numComments;
	MYSQL *conn;
	int nextPage;
	char nextStr[MAX_URL_LENGTH];
	char prevStr[MAX_URL_LENGTH];
	char json[COMMENT_JSON_LENGTH];
	char commentBuffer[buffSize];
	int i;

	/*Parse */
	commentPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_comment));
	if(commentPage == NULL){
		return -1; /* Return flag to send self to cc_nomem */
	}

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		free(commentPage);
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}

	bzero(nextStr, sizeof nextStr);
	bzero(prevStr, sizeof prevStr);
	bzero(commentBuffer,buffSize);

	if(cType == NULL)
		numComments = db_getComments(page, _shared_campaign_id ,commentPage, conn);
	else
		numComments = db_getCommentsByType(page, _shared_campaign_id, commentPage, cType, conn);

	if( numComments > RESULTS_RETURNED ){
		nextPage = page+1;
		if(cType == NULL)
			snprintf(nextStr,MAX_URL_LENGTH, "%scomments?page=%d", BASE_API_URL, nextPage);
		else
			snprintf(nextStr,MAX_URL_LENGTH, "%scomments?page=%d&type=%s", BASE_API_URL, nextPage, cType);
	} else {
		snprintf(nextStr,MAX_URL_LENGTH, "null");
	}

	if(page > 1)
		if(cType == NULL)
			snprintf(prevStr,MAX_URL_LENGTH,"%scomments?page=%d",BASE_API_URL,page-1);
		else
			snprintf(prevStr,MAX_URL_LENGTH,"%scomments?page=%d&type=%s",BASE_API_URL,page-1,cType);
	else
		snprintf(prevStr,MAX_URL_LENGTH,"null");

	/* Build the actual list of JSON objects for the comments
	 * use the buffer itself as our store and hope for the best.
	 * note the use of min here is neccesary. We grab RESULTS_RETURNED+1 
	 * from the database to check the next page. So we need that -1 or
	 * the number we've actually been returned
	*/
	for(i=0; i < min(numComments,RESULTS_RETURNED); ++i){
		bzero(json,sizeof json);
		gs_commentToNJSON(commentPage[i],json,sizeof json);
		/* This could be done more efficiently
		 * by performing the string cpy manually and saving the place of 
		 * the end of the string to continue writing
		*/
		if(i==0)
			snprintf(commentBuffer,buffSize,"%s",json);
		else{
			strncat(commentBuffer,",",buffSize);
			strncat(commentBuffer,json,buffSize);
		}			
	}

	free(commentPage);
	snprintf(buffer,buffSize, COMMENT_PAGE_STR, 200, commentBuffer, min(numComments,RESULTS_RETURNED), page+1, nextStr,prevStr);
	
	mysql_close(conn);
	mysql_thread_end();
	return 0;
}

int comment_post(char * buffer, int buffSize, const struct http_request * request){
	MYSQL *conn;
	struct gs_comment insComment;
	StrMap * sm;
	int i;
	int j;
	int strFlag;
	char keyBuffer[GS_COMMENT_MAX_LENGTH+1];
	char valBuffer[GS_COMMENT_MAX_LENGTH+1];


	bzero(keyBuffer,sizeof keyBuffer);
	gs_comment_ZeroStruct(&insComment);
	strFlag = 0;

	fprintf(stderr, "Called Comment Post with: B:%s L:%d P:%p\n", buffer, buffSize, request->data );

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		fprintf(stderr, "sm err\n");
		return -1;
	}

	/*Parse the JSON for the information we desire */
	for(i=0; i < buffSize && request->data[i] != '\0'; ++i){
		/*We're at the start of a string*/
		if(request->data[i] == '"'){
			/*Go until we hit the closing qoute*/
			i++;
			for(j=0; i < buffSize && request->data[i] != '\0' && request->data[i] != '"' && (unsigned int)j < sizeof keyBuffer; ++j,++i){
				keyBuffer[j] = (int)request->data[i] > 64 && request->data[i] < 91 ? request->data[i] + 32 : request->data[i];
			}
			keyBuffer[j] = '\0';
			/*find the beginning of the value
			 *which is either a " or a number. So skip spaces and commas
			*/
			for(i++; i < buffSize && request->data[i] != '\0' && (request->data[i] == ',' || request->data[i] == ' ' || request->data[i] == ':'); ++i)
				;
			/*Skip any opening qoute */
			if(request->data[i] != '\0' && request->data[i] == '"'){
				i++;
				strFlag = 1;
			}
			for(j=0; i < buffSize && request->data[i] != '\0'; ++j,++i){
				if(strFlag == 0){
					if(request->data[i] == ' ')
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

	/* Determine if the request is valid or not */
	if(sm_exists(sm, "type") !=1 || sm_exists(sm, "message") !=1){
		sm_delete(sm);
		fprintf(stderr, "required keys not found\n");
		return -400;		
	}else{
		if(sm_exists(sm, "type") ==1){
			if(sm_get(sm,"type", valBuffer, sizeof valBuffer) == 1){
				/* Verify that it is a correct type */
				if(strncasecmp(valBuffer, CTYPE_1,COMMENTS_CTYPE_SIZE) != 0)
					if(strncasecmp(valBuffer, CTYPE_2,COMMENTS_CTYPE_SIZE) != 0)
						if(strncasecmp(valBuffer, CTYPE_3,COMMENTS_CTYPE_SIZE) != 0){				
							sm_delete(sm);
							return 400;
						}

			}
		}
	}

	/* valid, cary on and copy over */
	gs_comment_setScopeId(_shared_campaign_id, &insComment);
	sm_get(sm, "message",valBuffer, sizeof valBuffer);
	gs_comment_setContent(valBuffer, &insComment);
	if(sm_exists(sm, "pin")){
		sm_get(sm, "pin",keyBuffer,sizeof keyBuffer);
		gs_comment_setPinId(atol(keyBuffer),&insComment);
	}
	sm_get(sm, "type", insComment.cType, sizeof insComment.cType);
	sm_delete(sm);



	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}

	/* Insert the comment */
	db_insertComment(&insComment, conn);

	mysql_close(conn);
	mysql_thread_end();

	/* populate the response */
	snprintf(buffer,buffSize,"{\"status_code\" : 200,\"message\" : \"Succesfully submited new comment\"}");

	return 200;
}


int comment_delete(char * buffer, int buffSize, long id){
	MYSQL *conn;
	long affected; 

	mysql_thread_init();
	conn = _getMySQLConnection();
	if(!conn){
		mysql_thread_end();
		fprintf(stderr, "%s\n", "Could not connect to mySQL on worker thread");
		return -1;
	}	

	affected = db_deleteComment(id,conn);

	mysql_close(conn);
	mysql_thread_end();

	if(affected > 0){
		snprintf(buffer,buffSize,"{\"status_code\" : 204,\"message\" : \"Successfuly deleted comment\"}");
		return 204;
	} else {
		snprintf(buffer,buffSize,"{\"status_code\" : 404,\"message\" : \"Could not find comment with given id\"}");
		return 404;
	}
}