#include <unistd.h>
#include "controllers/markers.h"
#include "network/router.h"


#define EXPECTED(expected, status, errmessage) \
	if(expected == status) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "PUT PINS: Expected status of %d, recieved %d. %s (%s::%d)\n", expected,status, errmessage,__FILE__,__LINE__ );\
	}


int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	struct gs_marker testMarker;
	struct gs_comment testComment;
	Decimal latitude;
   	Decimal longitude;
   	MYSQL * conn;
   	char * data = "{\"addressed\" : true}";

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}
	

	latitude = createDecimalFromString( "-44.050");
	longitude= createDecimalFromString( "-44.70");

   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);
	   
   	db_insertComment(&testComment,conn);

   	latitude = createDecimalFromString( "-44.050");
	longitude= createDecimalFromString( "-44.70");

   
	gs_marker_ZeroStruct(&testMarker);

	gs_marker_setCommentId(1, &testMarker);
	gs_marker_setScopeId(CAMPAIGN_ID, &testMarker);
	gs_marker_setLongitude(longitude, &testMarker);
	gs_marker_setLatitude(latitude, &testMarker);

	db_insertMarker(&testMarker, conn);

	
	
	stringToReturn = malloc(1000);
	request.method = PUT;
	request.data = data;
	request.contentLength = strlen(data);
	
	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(200,status, "Expected valid request")
	
	/* Invalids */	
	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(404,status, "Expected marker to not be found")

	sprintf(request.url, "/api/pins");
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "Expected invalid when not submitting an id parameter")

	sprintf(request.url, "/api/pins?id=derp");
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "Expected valid request")

	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	request.data = "";
	request.contentLength = 0;
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "Expected invalid response because of malformed JSON")

	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	request.data = "{}";
	request.contentLength = 3;
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "Expected invalid response because of lack of addressed key")

	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED