#include "controllers/markers.h"

int marker_controller(const struct http_request * request, char * stringToReturn, int strLength){
	fprintf(stderr, "Working with %p %s %d", (void*)request, stringToReturn, strLength);
	return 503;
}