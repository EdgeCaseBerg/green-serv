#define GREEN_SERV 1
#include <unistd.h>
#include "controllers/heatmaps.h"
#include "network/router.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "GET HEATMAP: Expected status of %d, recieved %d. %s (%s::%d)\n%s\n", expected,status, errmessage, __FILE__, __LINE__, stringToReturn );\
	}

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	int size = 1000;
	stringToReturn = malloc(size);
	request.method = GET;
	_shared_campaign_id = 1;
	
	sprintf(request.url, "/api/heatmap");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Failed on default heatmap")

	/* Use of optional parameters */
	sprintf(request.url, "/api/heatmap?latDegrees=4");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for latDegrees=4" );
	

	sprintf(request.url, "/api/heatmap?lonDegrees=6");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for lonDegrees=6")
	

	sprintf(request.url, "/api/heatmap?precision=4");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for precision=4")
	

	sprintf(request.url, "/api/heatmap?raw=true");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for raw=true")


	sprintf(request.url, "/api/heatmap?raw=false");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for raw=false")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4&latDegrees=5");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried with latDegrees and lonDegrees")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for lonDegrees and offsets")
	

	sprintf(request.url, "/api/heatmap?latDegrees=4s&lonOffset=1&latOffset=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "latDegrees and offsets")


	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for both degrees and offsets")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6&page=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried with degrees, offsets, and page")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(200, status, "Queried for degrees, offsets, and precision")


	/* Invalid GETs */
	sprintf(request.url, "/api/heatmap?latDegrees=-92");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "latDegrees out of bounds (negative)")
	

	sprintf(request.url, "/api/heatmap?latDegrees=92");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "latDegrees out of bounds (positive)")
	
	
	sprintf(request.url, "/api/heatmap?lonDegrees=-192");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "lonDegrees out of bounds (negative")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=192");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "lonDegrees out of bounds (positive)")
	

	sprintf(request.url, "/api/heatmap?latDegrees=nasn");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(400, status, "non-numeric latDegrees")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=nan");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(400, status, "non-numeric lonDegrees")
	

	sprintf(request.url, "/api/heatmap?precision=nan");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(400, status, "non-numeric precision")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "Queried with just one offset (lat)")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&lonOffset=1&latDegrees=6&precision=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(422, status, "Queried with just one offset (lng")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=a&latDegrees=6&precision=1&lonOffset=1");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(400, status, "Queried with non numeric offset (lat)")
	

	sprintf(request.url, "/api/heatmap?lonDegrees=4s&latOffset=3&latDegrees=6&precision=1&lonOffset=a");
	status = heatmap_controller(&request, stringToReturn, size);
	EXPECTED(400, status, "Queried with non-numeric offset (lng")
	
		
	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED