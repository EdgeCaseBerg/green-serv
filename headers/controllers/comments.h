#ifndef __COMMENTS_CONTROLLER_H__
	#define __COMMENTS_CONTROLLER_H__
	
	#ifndef COMMENTS_CTYPE_SIZE
	#define COMMENTS_CTYPE_SIZE 10
	#endif
	
	#include "helpers/strmap.h"
	#include "network/router.h"
	#include "models/comment.h"
	#include "db.h"
	#include "helpers/json.h"
	
	#include <stdio.h>
	#include <stdlib.h>
	
	#ifndef CTYPE_1
		#define CTYPE_1 "COMMENT"
		#define CTYPE_2 "ADMIN"
		#define CTYPE_3 "MARKER"
	#endif
	
	#ifndef NOMEM_ERROR
		#define NOMEM_ERROR "Request could not be processed due to memory allocation failure"
	#endif
	#ifndef BAD_PAGE_ERR
		#define BAD_PAGE_ERR "Page must be a non zero, positive integral value"
	#endif
	#ifndef BAD_TYPE_ERR
		#define BAD_TYPE_ERR "Type must be of " CTYPE_1 "," CTYPE_2 ", or " CTYPE_3
	#endif
	#ifndef BAD_METHOD_ERR
		#define BAD_METHOD_ERR "Request method not supported"
	#endif
	#define MISSING_KEY_ERR "Request body does not have all required keys of type and message"
	#ifndef MISSING_ID_KEY
		#define MISSING_ID_KEY "Required key id not found"
	#endif

	int comment_controller(const struct http_request * request, char * stringToReturn, int strLength);

	int comments_get(char * buffer, int buffSize,int page,char * cType);

	int comment_post(char * buffer, int buffSize, const struct http_request * request);

	int comment_delete(char * buffer, int buffSize, long id);

	#define COMMENT_PAGE_STR	"{" \
									"\"status_code\" : %d ,"\
									"\"comments\" : ["\
									"%s"\
									"],"\
									"\"page\" : {"\
											"\"count\" : %d,"\
											"\"index\" : %d,"\
											"\"next\" : \"%s\","\
											"\"previous\" : \"%s\""\
										"}"\
								"}"


#endif