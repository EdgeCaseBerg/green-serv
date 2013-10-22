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
	
	/* The default offset for lat and lon */
	#define DEFAULT_OFFSET "0.5"

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
	#ifndef MISSING_ID_KEY
		#define MISSING_ID_KEY "Required key or value for id not present"
	#endif
	#ifndef NO_PUT_DATA
		#define NO_PUT_DATA "No data sent with request."
	#endif
	#define BAD_LON_OFFSET "Longitude offset must be numeric"
	#define BAD_LAT_OFFSET "Latitude offset must be numeric"
	#define NO_ADDRESSED_KEY "Required key or value for addressed not present"

	int marker_controller(const struct http_request * request, char * stringToReturn, int strLength);

	int marker_delete(char * buffer, int buffSize, long id);

	int marker_address(char * buffer, int buffSize, long id, const struct http_request * request);

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

