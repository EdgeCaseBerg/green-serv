#ifndef __REPORTS_CONTROLLER_H__
	#define __REPORTS_CONTROLLER_H__
	
	#include "helpers/strmap.h"
	#include "network/router.h"
	#include "models/report.h"
	#include "db.h"
	#include "helpers/json.h"
	#include "controllers/macros.h"
	
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	
	#ifndef NOMEM_ERROR
		#define NOMEM_ERROR "Request could not be processed due to memory allocation failure"
	#endif
	#ifndef BAD_PAGE_ERR
		#define BAD_PAGE_ERR "Page must be a non zero, positive integral value"
	#endif
	#ifndef BAD_PAGE_SINCE_ERR
		#define BAD_PAGE_SINCE_ERR "Page and hash parameters are mutually exclusive"
	#endif	
	#ifndef	BAD_FORMAT_DATE_ERR
		#define BAD_FORMAT_DATE_ERR "The Since datetime format could not be parsed, please use YYYY-mm-dd HH:MM format"
	#endif
	#ifndef HASH_ORIGIN_REQUIRED_ERR
		#define HASH_ORIGIN_REQUIRED_ERR "Both hash and origin parameters are required"
	#endif
	#ifndef BAD_METHOD_ERR
		#define BAD_METHOD_ERR "Request method not supported"
	#endif
	#ifndef EMPTY_MESSAGE__ERR
		#define EMPTY_MESSAGE_ERR "Field message may not be empty"
	#endif
	#ifndef EMPTY_TRACE__ERR
		#define EMPTY_TRACE_ERR "Field stackTrace may not be empty"
	#endif
	#ifndef EMPTY_ORIGIN__ERR
		#define EMPTY_ORIGIN_ERR "Field origin may not be empty"
	#endif
	#ifndef KEYS_MISSING
		#define KEYS_MISSING "Could not process request due to required keys not being found in data."
	#endif
	#ifndef REPORT_NOT_FOUND
		#define REPORT_NOT_FOUND "Could not find report"
	#endif
	#ifndef ORIGIN_NOT_ALLOWED
		#define ORIGIN_NOT_ALLOWED "You do not have permission to delete this report"
	#endif
	#ifndef MALFORMED_JSON
		#define MALFORMED_JSON "Could not parse JSON"
	#endif
	#ifndef ORIGIN_REQUIRED
		#define ORIGIN_REQUIRED "Origin parameter must be present"
	#endif
	#ifndef HASH_REQUIRED
		#define HASH_REQUIRED "Hash parameter must be present"
	#endif
	#ifndef BAD_REPORT_TYPE_ERR
		#define BAD_REPORT_TYPE_ERR "Report types must be one of the following: " REPORT_TYPES
	#endif

	int report_controller(const struct http_request * request, char ** stringToReturn, int strLength);

	int report_delete(char * buffer, int buffSize, char * origin, char * hash);

	int report_post(char * buffer, int buffSize, const struct http_request * request);

	int report_get(char * buffer,int buffSize, char * hash, char * since, int page);

	#define REPORT_PAGE_STR	"{" \
								"\"status_code\" : %d ,"\
								"\"messages\" : ["\
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