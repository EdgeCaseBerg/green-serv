#include <unistd.h>
#include "controllers/reports.h"
#include "network/router.h"

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;

	
	sprintf(request.url, "/api/debug");
	stringToReturn = malloc(1000);
	request.method = PUT;
	/* Expect invalid method */
	status = report_controller(&request, &stringToReturn, 1000);
	if( status == 501 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "PUT DEBUG: Request failed to return invalid status on invalid Method, returned: %d\n", status );	
	}
		

	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}