#include "controllers/comments.h"


/*Internal Routing for the comment controller.
 *Responsible to delegate to GET,POST,PUT,DELETE methods here.
 *Will return whatever the result of the request is into the 
 *stringToReturn. Will NOT overflow the buffer. If not enough space
 *is given controller may return an invalid response.
 *Returns the status code of the response.
*/
int comment_controller(const struct http_request * request, char * stringToReturn, int strLength){
	struct gs_comment * commentPage;
	char cType[COMMENTS_CTYPE_SIZE];
	char buffer[(RESULTS_PER_PAGE * sizeof(struct gs_comment))*4+1];
	char tempBuf[20];
	int page;
	int status;
	int numParams;
	StrMap * sm;

	page=1;
	status = 503;
	bzero(cType, sizeof cType);
	bzero(buffer, sizeof buffer);
	bzero(tempBuf, sizeof tempBuf);

	/*Parse */
	commentPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_comment));
	if(commentPage == NULL){
		status = 500;
		goto cc_nomem;
	}

	sm = sm_new(HASH_TABLE_CAPACITY);
	if(sm == NULL){
		status = 500;
		free(commentPage);
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
					free(commentPage);
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
							free(commentPage);
							sm_delete(sm);
							goto cc_badtype;
						}

			}
		}else{
			/*No Type, set flag*/
			cType[0] = '\0';
		}
	}

	switch(request->method){
		case GET:
			if(cType[0] == '\0' )
				comments_get(buffer, sizeof buffer ,page,NULL);
			else
				comments_get(buffer, sizeof buffer ,page,cType);
			break;
		case POST:
			break;
		default:
			/*Invalid Method Err*/
			status = 501;
			free(commentPage);
			sm_delete(sm);
			goto cc_unsupportedMethod;
			break;
	}

	free(commentPage);
	sm_delete(sm);

	return status;

	cc_nomem: /*Comment Controller Memory Allocation fail */
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, NOMEM_ERROR);
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

}

/*Accepts NULL for cType if no filter
 *page will default to 1 if negative values passed in or 0
 *
*/
int comments_get(char * buffer, int buffSize,int page,char * cType){
	fprintf(stderr, "Called Comment Get with: B:%s L:%d P:%d T:%p\n", buffer, buffSize, page, cType );
	return -1;
}

int comment_post(char * buffer, int buffSize, const struct http_request * request){
	fprintf(stderr, "Called Comment Post with: B:%s L:%d P:%p\n", buffer, buffSize, request->data );
	return -1;
}