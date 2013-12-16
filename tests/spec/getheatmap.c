#include <unistd.h>
#include "controllers/heatmaps.h"
#include "network/router.h"

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;

	request.method = GET;
	sprintf(request.url, "/api/heatmap");
	
	stringToReturn = malloc(1000);

	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200\n" );
	}
	
	/* Use of optional parameters */
	sprintf(request.url, "/api/heatmap?latDegrees=4");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for latDegrees=4, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=6");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for lonDegrees=6, returned %d\n", status );
	}	

	sprintf(request.url, "/api/heatmap?precision=4");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for precision=4, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?raw=true");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 for raw=true, returned %d\n", status );
	}


	sprintf(request.url, "/api/heatmap?raw=false");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for raw=false, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4&latDegrees=5");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried with latDegrees and lonDegrees, returned %d\n", status );
	}	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for lonDegrees and offsets, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?latDegrees=4s&lonOffset=1&latOffset=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for latDegrees and offsets, returned %d\n", status );
	}


	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for degrees and offsets, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6&page=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried with degrees, offsets, and page, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 200 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 200 when queried for degrees,offsets, and precision, returned %d\n", status );
	}



	/* Invalid GETs */
	sprintf(request.url, "/api/heatmap?latDegrees=-92");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 for latDegrees of out bounds, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?latDegrees=92");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 for latDegrees of out bounds, returned %d\n", status );
	}
	
	sprintf(request.url, "/api/heatmap?lonDegrees=-192");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 for lonDegrees of out bounds, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=192");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 for lonDegrees of out bounds, returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?latDegrees=nan");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 400 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 400 for non-numeric latDegrees , returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=nan");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 400 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 400 for non-numeric latDegrees , returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?precision=nan");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 400 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 400 for non-numeric precision , returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 when queried with just one offset (lat), returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 422 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 422 when queried with just one offset (lng), returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=a&latDegrees=6&precision=1&lonOffset=1");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 400 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 400 when queried with non numeric offset (lat), returned %d\n", status );
	}

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=3&latDegrees=6&precision=1&lonOffset=a");
	status = heatmap_controller(&request, stringToReturn, 1000);
	if( status == 400 )
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "GET heatmap: request failed to return status of 400 when queried with non numeric offset (lng), returned %d\n", status );
	}

		

	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}