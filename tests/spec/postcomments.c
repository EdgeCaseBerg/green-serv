#include "config.h"
#include <unistd.h>
#include "controllers/comments.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "POST COMMENT: Expected status of %d, recieved %d. %s (%s::%d)\n", expected,status, errmessage, __FILE__, __LINE__ );\
	}

#define SETDATA(datum) \
	request.data = datum;\
	request.contentLength = strlen(datum);

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;
	struct gs_marker testMarker; 
	Decimal latitude;
   	Decimal longitude;

	char * valid1 = "{\"type\":\"COMMENT\", \"message\":\"test message from post comments\"}";
	char * valid2 = "{\"type\":\"MARKER\", \"message\":\"test message from post comments\"}";
	char * valid3 = "{\"type\":\"ADMIN\", \"message\":\"test message from post comments\"}";
	char * validWithPinTemplate = "{\"type\":\"MARKER\", \"message\":\"test message with pin id\", \"pin\": ";
	char * validWithPin = malloc(256);

	char * invalid = "{";
	char * invalidKey = "{\"tpe\":\"COMMENT\", \"message\":\"test message from post comments\"}";
	char * invalidNull = "{\"type\":\"COMMENT\", \"message\":}";
	char * invalidType = "{\"type\":\"Invalid\", \"message\":\"test message from post comments\"}";
	char * invalidPin = "{\"type\":\"COMMENT\", \"message\":\"test message from post comments\", \"pin\" : abc}";
	char * emptyMsg = "{\"type\":\"COMMENT\", \"message\":\"\"}";
	char * msgTooLarge = "\"type\":\"COMMENT\", \"message\":\"abcdefghijklmnopqrstuvwxyz01234567890abcdefghijklmnopqrstuvwxyz01234567890abcdefghijklmnopqrstuvwxyz01234567890abcdefghijklmnopqrstuvwxyz01234567890\"}";


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
   
	gs_marker_ZeroStruct(&testMarker);
	gs_marker_setCommentId(1, &testMarker);
	gs_marker_setScopeId(CAMPAIGN_ID, &testMarker);
	gs_marker_setLongitude(longitude, &testMarker);
	gs_marker_setLatitude(latitude, &testMarker);
	db_insertMarker(&testMarker, conn);



	sprintf(validWithPin,"%s%ld}" , validWithPinTemplate, testMarker.id);
   	

   	stringToReturn = malloc(1000);
	request.method = POST;
	
	/* Valids */
	sprintf(request.url, "/api/comments");
	SETDATA(valid1)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request failed to return no content")

	sprintf(request.url, "/api/comments");
	SETDATA(valid2)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request failed to return no content")

	sprintf(request.url, "/api/comments");
	SETDATA(valid3)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request failed to return no content")

	sprintf(request.url, "/api/comments");
	SETDATA(validWithPin)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Request failed to return no content")

	/* Invalids */
	sprintf(request.url, "/api/comments");
	SETDATA(invalid)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Expected malformed request")

	sprintf(request.url, "/api/comments");
	SETDATA(invalidKey)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected key error")

	sprintf(request.url, "/api/comments");
	SETDATA(invalidNull)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected Err for empty message or non-null message")

	sprintf(request.url, "/api/comments");
	SETDATA(invalidType)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected rejection on invalid type")

	sprintf(request.url, "/api/comments");
	SETDATA(invalidPin)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected failure for non-numeric pin id")

	sprintf(request.url, "/api/comments");
	SETDATA(emptyMsg)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected failure for having an empty message")

	sprintf(request.url, "/api/comments");
	SETDATA(msgTooLarge)
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Expected failure for message being too large")

	
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