#ifndef __MARKERS_CONTROLLER_H__
	#define __MARKERS_CONTROLLER_H__
	
	#include <stdio.h>
	#include <stdlib.h>

	#include "helpers/strmap.h"
	#include "network/router.h"
	#include "db.h"
	#include "models/marker.h"
	
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

	int marker_controller(const struct http_request * request, char * stringToReturn, int strLength);

	#define MARKER_PAGE_STR	"{" \
								"\"status_code\" : %d ,"\
								"\"pins\" : ["\
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

