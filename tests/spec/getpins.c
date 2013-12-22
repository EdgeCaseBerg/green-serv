#include <unistd.h>
#include "controllers/comments.h"
#include "network/router.h"

#define EXPECTED(expected, status, errmessage) \
	if(expected == status) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "GET PINS: Expected status of %d, recieved %d. %s (%s::%d)\n", expected,status, errmessage,__FILE__,__LINE__ );\
	}


int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	int size;

	size = 1000;
	request.method = GET;
	stringToReturn = malloc(size);


	sprintf(request.url, "/api/pins");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on non-parameterized pins get.")

	sprintf(request.url, "/api/pins?latDegrees=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on latDegrees=1.")	
			
	sprintf(request.url, "/api/pins?lonDegrees=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on lonDegrees=1.")	

	sprintf(request.url, "/api/pins?latDegrees=1&lonDegrees=2");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on latDegrees=1 and lonDegrees=2.")	

	sprintf(request.url, "/api/pins?latDegrees=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on latDegrees=1.")	

	sprintf(request.url, "/api/pins?latDegrees=1&latOffset=1&lonOffset=2");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Failed on latDegrees=1.")	

	/* Invalids */
	sprintf(request.url, "/api/pins?latDegrees=-91");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422,status, "Failed on out of bounds latDegrees from latDegrees=-91")	

	sprintf(request.url, "/api/pins?latDegrees=91");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422,status, "Expected out of bounds result from latDegrees=91")	

	sprintf(request.url, "/api/pins?lonDegrees=191");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422,status, "Expected out of bounds result from lonDegrees=191")	

	sprintf(request.url, "/api/pins?latDegrees=-192");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422,status, "Expected out of bounds result from latDegrees=-192")	

	sprintf(request.url, "/api/pins?latDegrees=a");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(400,status, "Should have Failed on non-numeric latDegrees=a.")	

	sprintf(request.url, "/api/pins?lonDegrees=b");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Should have failed on non-numeric lonDegrees=b.")	

	sprintf(request.url, "/api/pins?latOffset=a&lonOffset=3");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(400,status, "Should have Failed on non-numeric latOffset=a.")	

	sprintf(request.url, "/api/pins?&latOffset=2lonOffset=b");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Should have failed on non-numeric lonOffset=b.")	

	sprintf(request.url, "/api/pins?latOffset=5");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(400,status, "Should have Failed with only one offset")	

	sprintf(request.url, "/api/pins?lonOffset=12");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(400,status, "Should have Failed with only one offset")	


	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED