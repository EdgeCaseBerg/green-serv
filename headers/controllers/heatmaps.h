#ifndef __HEATMAP_CONTROLLER_H__
	#define __HEATMAP_CONTROLLER_H__
		
	#include "helpers/strmap.h"
	#include "network/router.h"
	#include "models/heatmap.h"
	#include "db.h"
	#include "helpers/json.h"
	#include "helpers/mlist.h"
	#include "controllers/macros.h"
	
	#include <stdio.h>
	#include <stdlib.h>
	
	#ifndef NOMEM_ERROR
		#define NOMEM_ERROR "Request could not be processed due to memory allocation failure"
	#endif
	#ifndef BAD_PAGE_ERR
		#define BAD_PAGE_ERR "Page must be a non zero, positive integral value"
	#endif
	#ifndef BAD_METHOD_ERR
		#define BAD_METHOD_ERR "Request method not supported"
	#endif
	#ifndef MISSING_KEY_ERR
		#define MISSING_KEY_ERR "Request body does not have all required keys of type and message"
	#endif
	#ifndef BAD_LON_OFFSET
		#define BAD_LON_OFFSET "Longitude offset must be numeric"
	#endif
	#ifndef BAD_LAT_OFFSET
		#define BAD_LAT_OFFSET "Latitude offset must be numeric"
	#endif
	#ifndef BOTH_OFFSET_ERR
		#define BOTH_OFFSET_ERR "Both lonOffset and latOffset must be present if either is used"
	#endif
	#ifndef BAD_INTENSITY_ERR
		#define BAD_INTENSITY_ERR "Intensity must be a non-negative integral value"
	#endif
	#ifndef BAD_INTENSITY_NEG_ERR
		#define BAD_INTENSITY_NEG_ERR "Intensity must be a non-negative unsigned integral value"
	#endif
	#ifndef LATITUDE_OUT_OF_RANGE_ERR
		#define LATITUDE_OUT_OF_RANGE_ERR "Latitude must be within -90 and 90 degrees"
	#endif
	#ifndef LONGITUDE_OUT_OF_RANGE_ERR
		#define LONGITUDE_OUT_OF_RANGE_ERR "Longitude must be within -180 and 180 degrees"
	#endif
	#ifndef KEYS_MISSING
		#define KEYS_MISSING "Could not process request due to required keys not being found in data."
	#endif
	#ifndef PRECISION_ERR
		#define PRECISION_ERR "Precision must be an unsigned non-zero integral number"
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef TRUE
		#define TRUE 1
	#endif
	#ifndef DEFAULT_OFFSET
		#define DEFAULT_OFFSET "0.5"
	#endif

	int heatmap_controller(const struct http_request * request, char * stringToReturn, int strLength);

	int heatmap_get(char * buffer, int buffSize,int page, Decimal * latDegrees, Decimal * latOffset, Decimal * lonDegrees, Decimal * lonOffset, int precision, int raw);

	int heatmap_put(char * buffer, int buffSize, const struct http_request * request);


	#define HEATMAP_PAGE_STR	"{" \
									"\"status_code\" : %d ,"\
									"\"grid\" : ["\
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