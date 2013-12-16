#include <unistd.h>
#include "controllers/comments.h"
#include "network/router.h"

#define EXPECTED(expected, status, errmessage) \
	if(expected == status) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "GET PINS: Expected status of %d, recieved %d. %s\n", expected,status, errmessage );\
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
			

	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED