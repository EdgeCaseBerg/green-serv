#define GREENSERV 1
#include "config.h"
#include <unistd.h>
#include "controllers/markers.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "POST PINS: Expected status of %d, recieved %d. %s (%s::%d)\n%s\n", expected,status, errmessage, __FILE__, __LINE__, stringToReturn );\
	}

#define SETDATA(datum) \
	request.data = datum;\
	request.contentLength = strlen(datum);

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;

	_shared_campaign_id =1 ;

	char * valid1 = "{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"type\" : \"ADMIN\",\"message\" : \"I had to run to feed my cat, had to leave my Trash here sorry! Can someone pick it up?\",\"addressed\" : true}";
	char * valid2 = "{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"type\" : \"MARKER\",\"message\" : \"I had to run to feed my cat, had to leave my Trash here sorry! Can someone pick it up?\",\"addressed\" : true}";
	char * valid3 = "{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"type\" : \"COMMENT\",\"message\" : \"I had to run to feed my cat, had to leave my Trash here sorry! Can someone pick it up?\",\"addressed\" : true}";


	char * malformed = "{{ }} {{ } { {{ }";
	char * keysMissing = "{\"latDegrees\" : 23, \"missing\":43, \"type\" : \"COMMENT\", \"message\" :\"msg\", \"addressed\" : false }";
	char * invalidType = "{\"latDegrees\" : 23, \"lonDegrees\":43, \"type\" : \"invalid\", \"message\" :\"msg\", \"addressed\" : false }";
	char * oobLat = "{\"latDegrees\" : 223, \"lonDegrees\":43, \"type\" : \"COMMENT\", \"message\" :\"msg\", \"addressed\" : false }";
	char * nanLat = "{\"latDegrees\" : abc, \"lonDegrees\":43, \"type\" : \"COMMENT\", \"message\" :\"msg\", \"addressed\" : false }";
	char * oobLon = "{\"latDegrees\" : 23, \"lonDegrees\":443, \"type\" : \"COMMENT\", \"message\" :\"msg\", \"addressed\" : false }";
	char * nanLon = "{\"latDegrees\" : 23, \"lonDegrees\": abc, \"type\" : \"COMMENT\", \"message\" :\"msg\", \"addressed\" : false }";
	char * pinMsg = "{\"latDegrees\" : 23, \"lonDegrees\": 43, \"type\" : \"COMMENT\", \"message\" :\"\", \"addressed\" : false }";

	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}
   	
   	stringToReturn = malloc(1000);
	request.method = POST;
	
	/* Valids */
	sprintf(request.url, "/api/pins");
	SETDATA(valid1)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request should have succeeded with a positive response")

	sprintf(request.url, "/api/pins");
	SETDATA(valid2)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request should have succeeded with a positive response")

	sprintf(request.url, "/api/pins");
	SETDATA(valid3)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request should have succeeded with a positive response")

	
	/* Invalids */
	sprintf(request.url, "/api/pins");
	SETDATA(malformed)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Should have failed with malformed error")

	sprintf(request.url, "/api/pins");
	SETDATA(keysMissing)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Should have failed when sent request with missing keys")

	sprintf(request.url, "/api/pins");
	SETDATA(invalidType)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Request should have failed when invalid type was given")

	sprintf(request.url, "/api/pins");
	SETDATA(oobLat)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed when latitude was out of bounds")

	sprintf(request.url, "/api/pins");
	SETDATA(nanLat)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Should have failed when latitude was not a number")

	sprintf(request.url, "/api/pins");
	SETDATA(oobLon)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed when longitude was out of bounds")

	sprintf(request.url, "/api/pins");
	SETDATA(nanLon)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Should have failed when longitude was not a number")

	sprintf(request.url, "/api/pins");
	SETDATA(pinMsg)
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed when pin message was empty")

	
	mysql_close(conn);
	mysql_library_end();
		
	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED
#undef SETDATA