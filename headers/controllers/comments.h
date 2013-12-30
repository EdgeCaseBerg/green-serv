#ifndef __COMMENTS_CONTROLLER_H__
	#define __COMMENTS_CONTROLLER_H__
	
	#ifndef COMMENTS_CTYPE_SIZE
	#define COMMENTS_CTYPE_SIZE 10
	#endif
	
	#include "helpers/strmap.h"
	#include "network/router.h"
	#include "models/comment.h"
	#include "controllers/macros.h"
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
	#define EMPTY_COMMENT_MESSAGE "Message may not be empty"
	#define MISSING_PIN_ERR "If pin key present, pin ID must be an integral numeric identifier"
	#ifndef MALFORMED_JSON
		#define MALFORMED_JSON "JSON Message cannot be interpreted due to malformed JSON"
	#endif
	#ifndef MESSAGE_TOO_LARGE
		#define MESSAGE_TOO_LARGE "Message may not be more than " STRINGIFY(GS_COMMENT_MAX_LENGTH)  " characters long."
	#endif
	#ifndef NAN_ID_KEY
		#define NAN_ID_KEY "ID must be a numeric identifier"
	#endif

	int comment_controller(const struct http_request * request, char * stringToReturn, int strLength);

	int comments_get(char * buffer, int buffSize, const struct http_request * request);

	int comment_post(char * buffer, int buffSize, const struct http_request * request);

	int comment_delete(char * buffer, int buffSize, const struct http_request * request);

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