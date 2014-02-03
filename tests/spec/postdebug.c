#define GREENSERV 1
#include "config.h"
#include <unistd.h>
#include "controllers/reports.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "POST DEBUG: Expected status of %d, recieved %d. %s (%s::%d)\n", expected,status, errmessage, __FILE__, __LINE__ );\
	}

#define SETDATA(datum) \
	request.data = datum;\
	request.contentLength = strlen(datum);



int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	/* Set Shared Campaign_id */
	_shared_campaign_id =1 ;
	
   	stringToReturn = malloc(1000);
	request.method = POST;

	char * valid = "{ \"message\" : \"Test Debug\",\"stackTrace\" : \"stack trace\",\"origin\" : \"6f3d78c8ca1cca4f362c697d9\"}";
	char * withType = "{ \"message\" : \"Test Debug\",\"stackTrace\" : \"stack trace\",\"origin\" : \"6f3d78c8ca1cca4f362c697d9\",\"type\":\"WARN\"}";

	char * malformed = "{ { {{} }aaa}";
	char * missingKeys = "{\"message\" : \"ms\"}";
	char * emptyMsg = "{ \"message\" : \"\",\"stackTrace\" : \"stack trace\",\"origin\" : \"6f3d78c8ca1cca4f362c697d9\"}";
	char * emptyTrace = "{ \"message\" : \"Test Debug\",\"stackTrace\" : \"\",\"origin\" : \"6f3d78c8ca1cca4f362c697d9\"}";
	char * emptyOrigin = "{ \"message\" : \"Test Debug\",\"stackTrace\" : \"stack trace\",\"origin\" : \"\"}";	
	char * invalidType = "{ \"message\" : \"Test Debug\",\"stackTrace\" : \"stack trace\",\"origin\" : \"6f3d78c8ca1cca4f362c697d9\",\"type\":\"WAsRN\"}";
	
	/* Valids */
	sprintf(request.url, "/api/debug");
	SETDATA(valid)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Expected successful request")

	SETDATA(withType)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(200, status, "Expected successful request")

	/* Invalids */
	sprintf(request.url, "/api/debug");
	SETDATA(malformed)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Expected failed request")

	sprintf(request.url, "/api/debug");
	SETDATA(missingKeys)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed when missing keys from JSON")

	sprintf(request.url, "/api/debug");
	SETDATA(emptyMsg)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed with empty message")	

	sprintf(request.url, "/api/debug");
	SETDATA(emptyTrace)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed with empty trace")

	sprintf(request.url, "/api/debug");
	SETDATA(emptyOrigin)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Should have failed when origin empty")

	SETDATA(invalidType)
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Expected failed request")	
		
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