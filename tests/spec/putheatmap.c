#define GREENSERV 1
#include <unistd.h>
#include "controllers/heatmaps.h"
#include "network/router.h"


#define EXPECTED(expected, status, errmessage) \
	if(expected == status) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "PUT HEATMAP: Expected status of %d, recieved %d. %s (%s::%d)\n%s\n", expected,status, errmessage,__FILE__,__LINE__, stringToReturn );\
	}

#define SETDATA(datum) \
	request.data = datum;\
	request.contentLength = strlen(datum);


int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;
	char * valid = "[{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"secondsWorked\" : 120}]";
	char * invalid = "[{{}]";
	char * invalidKeys = "[{\"Degrees\" : 24.53,\"lonDegrees\" : 43.2,\"secondsWorked\" : 120}]";
	char * nullData = "[{\"lonDegrees\" : null,\"latDegrees\" : null,\"secondsWorked\" : 120}]";
	char * latDegreesOOB = "[{\"latDegrees\" : 224.53,\"lonDegrees\" : 43.2,\"secondsWorked\" : 120}]";
	char * latDegreesNAN = "[{\"latDegrees\" : abc,\"lonDegrees\" : 43.2,\"secondsWorked\" : 120}]";
	char * lonDegreesOOB = "[{\"latDegrees\" : 24.53,\"lonDegrees\" : 243.2,\"secondsWorked\" : 120}]";
	char * lonDegreesNAN = "[{\"latDegrees\" : 24.53,\"lonDegrees\" : avc,\"secondsWorked\" : 120}]";
	char * negativeSeconds = "[{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"secondsWorked\" : -120}]";
	char * invalidSeconds = "[{\"latDegrees\" : 24.53,\"lonDegrees\" : 43.2,\"secondsWorked\" : derp}]";

	_shared_campaign_id =1 ;
	conn = _getMySQLConnection();
	if(!conn){
		fprintf(stderr, "%s\n", "Could not connect to mySQL");
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return 1;
	}	
	db_start_transaction(conn);

	stringToReturn = malloc(1000);
	request.method = PUT;	
	

	sprintf(request.url, "/api/heatmap");
	SETDATA(valid)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(200,status, "Expected valid request")
	
	/* Invalids */	
	sprintf(request.url, "/api/heatmap");
	SETDATA(invalid)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "Expected malformed request")

	sprintf(request.url, "/api/heatmap");
	SETDATA(invalidKeys)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "Expected invalid keys response")

	sprintf(request.url, "/api/heatmap");
	SETDATA(nullData)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "Cannot accept null data for required parameters")

	sprintf(request.url, "/api/heatmap");
	SETDATA(latDegreesOOB)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "latDegrees must be within the range of -90.0 and 90.")

	sprintf(request.url, "/api/heatmap");
	SETDATA(latDegreesNAN)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "latDegrees parameter must be numeric	")

	sprintf(request.url, "/api/heatmap");
	SETDATA(lonDegreesOOB)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "lonDegrees must be within the range of -180.0 and 180.")

	sprintf(request.url, "/api/heatmap");
	SETDATA(lonDegreesNAN)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "lonDegrees parameter must be numeric	")

	sprintf(request.url, "/api/heatmap");
	SETDATA(negativeSeconds)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(422,status, "secondsWorked should fail if given negative value")

	sprintf(request.url, "/api/heatmap");
	SETDATA(invalidSeconds)
	status = heatmap_controller(&request, stringToReturn, 1000);
	EXPECTED(400,status, "Seconds worked should fail if given non-numeric value")


	db_abort_transaction(conn);
	db_end_transaction(conn);
	free(stringToReturn);
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED