#include <unistd.h>
#include "controllers/comments.h"
#include "network/router.h"

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;

	
	sprintf(request.url, "/api/comments");
	stringToReturn = malloc(1000);
	request.method = PUT;
	/* Expect invalid method */
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 501 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 503 on invalid Method, returned: %d\n", status );	
	}
		

	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}