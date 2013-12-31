#define GREEN_SERV 1
#include "config.h"
#include <unistd.h>
#include "controllers/markers.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "DELETE PINS: Expected status of %d, recieved %d. %s (%s::%d)\n%s\n", expected,status, errmessage,__FILE__,__LINE__, stringToReturn );\
	}

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;
	struct gs_marker testMarker;
	struct gs_comment testComment;
	Decimal latitude;
   	Decimal longitude;


	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}

   	_shared_campaign_id =1 ;

   	latitude = createDecimalFromString( "-44.050");
	longitude= createDecimalFromString( "-44.70");

   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);
	   
   	db_insertComment(&testComment,conn);

   	latitude = createDecimalFromString( "-44.050");
	longitude= createDecimalFromString( "-44.70");

   
	gs_marker_ZeroStruct(&testMarker);

	gs_marker_setCommentId(testComment.id, &testMarker);
	gs_marker_setScopeId(CAMPAIGN_ID, &testMarker);
	gs_marker_setLongitude(longitude, &testMarker);
	gs_marker_setLatitude(latitude, &testMarker);

	db_insertMarker(&testMarker, conn);
   		
   	

   	stringToReturn = malloc(1000);
	request.method = DELETE;
	
	/* Valids */
	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(204, status, "Request failed to return no content")

	/* Invalids */
	sprintf(request.url, "/api/pins?id=%ld", testMarker.id);
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(404, status, "Expected not to find marker by id once its been deleted")

	sprintf(request.url, "/api/pins");
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected an err about id being a required field")

	sprintf(request.url, "/api/pins?id=derp");
	status = marker_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Request failed to err correctly when given non-numeric id")


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