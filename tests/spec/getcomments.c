#include <unistd.h>
#include "controllers/comments.h"
#include "network/router.h"

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;

	request.method = GET;
	sprintf(request.url, "/api/comments");
	
	stringToReturn = malloc(1000);

	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200\n" );
	}
	
	/* We could automate the requests below by putting the types into a loop 
	 * and then looping over them, but we're testing so we want to be as explicit
	 * as possible.
	*/
	sprintf(request.url, "/api/comments?type=COMMENT");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=comment\n" );
	}

	sprintf(request.url, "/api/comments?type=ADMIN");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=admin\n" );
	}	

	sprintf(request.url, "/api/comments?type=MARKER");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=marker\n" );
	}

	sprintf(request.url, "/api/comments?page=1");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200\n" );
	}


	sprintf(request.url, "/api/comments?type=COMMENT&page=1");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=comment\n" );
	}

	sprintf(request.url, "/api/comments?type=ADMIN&page=1");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=admin\n" );
	}	

	sprintf(request.url, "/api/comments?type=MARKER&page=1");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 200 when queried for type=marker\n" );
	}


	/* Test invalid GETs */
	sprintf(request.url, "/api/comments?type=bad");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 422 when queried with bad type\n" );
	}

	sprintf(request.url, "api/comments?page=-1");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 422 when queried with negative page\n" );
	}

	sprintf(request.url, "api/comments?page=derp");
	status = comment_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET COMMENTS: Plain failed to return status of 422 when queried with alphabetical page\n" );
	}


		

	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}