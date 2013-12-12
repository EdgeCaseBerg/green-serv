/* Depends on latDegrees being pointers to the Decimal type
 *
*/
#ifndef FREE_NON_NULL_DEGREES_AND_OFFSETS
	#define FREE_NON_NULL_DEGREES_AND_OFFSETS \
		if(latDegrees != NULL)\
			free(latDegrees);\
		if(lonDegrees != NULL)\
			free(lonDegrees); \
		if(latOffset != NULL)\
			free(latOffset);\
		if(lonOffset != NULL)\
			free(lonOffset);
#endif

/* This macro depends on stringToReturn being defined, strLength beind defined, ERROR_STR_FORMAT 
 * being defined, and a status being defined.  
 * char * stringToReturn, int strLength, int status
*/
#ifndef ERR_LABEL_STRING_TO_RETURN
	#define ERR_LABEL_STRING_TO_RETURN(label, errorStr) \
	label: \
		snprintf(stringToReturn, strLength, ERROR_STR_FORMAT, status, errorStr);\
		return status;
#endif