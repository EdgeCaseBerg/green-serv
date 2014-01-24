#ifndef __MARKERS_CONTROLLER_H__
	#define __MARKERS_CONTROLLER_H__
	
	#include <stdio.h>
	#include <stdlib.h>

	#include "helpers/strmap.h"
	#include "helpers/json.h"
	#include "network/router.h"
	#include "db.h"
	#include "models/marker.h"
	#include "models/comment.h"
	#include "controllers/comments.h" /* For comment error messages */
	#include "controllers/macros.h"
	
	#ifndef COMMENTS_CTYPE_SIZE
	#define COMMENTS_CTYPE_SIZE 10
	#endif

	#ifndef CTYPE_1
		#define CTYPE_1 "COMMENT"
		#define CTYPE_2 "ADMIN"
		#define CTYPE_3 "MARKER"
		#define CTYPE_4 "HAZARD"
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
		#define BAD_TYPE_ERR "Type must be of " CTYPE_1 "," CTYPE_2 "," CTYPE_4 ", or " CTYPE_3
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
	#ifndef KEYS_MISSING
		#define KEYS_MISSING "Could not process request due to required keys not being found in data."
	#endif
	#define BOTH_OFFSET_ERR "Both lonOffset and latOffset must be present if either is used"
	#define OOB_LATITUDE "Latitude must be between -91 and 91 (non-inclusive)"
	#define OOB_LONGITUDE "Longitude must be between -181 and 181 (non-inclusive)"
	#define NAN_LATITUDE "Latitude must be a number"
	#define NAN_LONGITUDE "Longitude must be a number"
	#define NAN_ID "id must be a numeric value"

	int marker_controller(const struct http_request * request, char * stringToReturn, int strLength);

	int marker_delete(char * buffer, int buffSize, long id);

	int marker_address(char * buffer, int buffSize, long id, const struct http_request * request);

	int marker_post(char * buffer, int buffSize, const struct http_request * request);

	int marker_get(char * buffer,int buffSize,Decimal * latDegrees, Decimal * lonDegrees, Decimal * latOffset,Decimal * lonOffset,int page);

	int marker_get_single(char * buffer,  int buffsize, long id);

	/* Although the version 3 API spec doesn't include the paging for pins
	 * the C API will return paging information since we enforce the limit
	 * on the pins through the MARKER_LIMIT constant defined in db.h
	*/
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

