#include <unistd.h>
#include "controllers/comments.h"
#include "network/router.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "GET COMMENTS: Expected status of %d, recieved %d. %s (%s::%i)\n", expected,status, errmessage, __FILE__, __LINE__ );\
	}

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	int size;

	size = 1000;
	request.method = GET;
	sprintf(request.url, "/api/comments");
	
	stringToReturn = malloc(size);

	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Default GET failed")
	
	/* We could automate the requests below by putting the types into a loop 
	 * and then looping over them, but we're testing so we want to be as explicit
	 * as possible.
	*/
	sprintf(request.url, "/api/comments?type=COMMENT");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=comment")
	

	sprintf(request.url, "/api/comments?type=ADMIN");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=admin")
	

	sprintf(request.url, "/api/comments?type=MARKER");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=marker")
	

	sprintf(request.url, "/api/comments?page=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200,status, "Queried for page=1")


	sprintf(request.url, "/api/comments?type=COMMENT&page=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=comment")
	

	sprintf(request.url, "/api/comments?type=ADMIN&page=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=admin")
	

	sprintf(request.url, "/api/comments?type=MARKER&page=1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for type=marker and page=1")


	/* Test invalid GETs */
	sprintf(request.url, "/api/comments?type=bad");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "Queried with bad type")
	

	sprintf(request.url, "api/comments?page=-1");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "Queried with negative page value")
	

	sprintf(request.url, "api/comments?page=derp");
	status = comment_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "Queried with alphabetical page instead of number")
			
	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED