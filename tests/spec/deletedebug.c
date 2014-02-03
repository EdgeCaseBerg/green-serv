#define GREEN_SERV 1
#include "config.h"
#include <unistd.h>
#include "controllers/reports.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "DELETE DEBUG: Expected status of %d, recieved %d. %s (%s::%d)\n%s\n", expected,status, errmessage,__FILE__,__LINE__, stringToReturn );\
	}

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;
	struct gs_report testReport;

	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}
   	_shared_campaign_id =1 ;
   		
   	gs_report_ZeroStruct(&testReport); 	

   	gs_report_setScopeId(CAMPAIGN_ID, &testReport);
	gs_report_setContent("cc obj/sha256temp.o -o sha256.o -lcrypto\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 0 has invalid symbol index 10\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 1 has invalid symbol index 11\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 2 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 3 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 4 has invalid symbol index 10\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 5 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 6 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 7 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 8 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 9 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 10 has invalid symbol index 11\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 11 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 12 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 13 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 14 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 15 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 16 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 17 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 18 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 19 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 20 has invalid symbol index 19\n/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu/crt1.o: In function `_start':\n(.text+0x20): undefined reference to `main'\ncollect2: ld returned 1 exit status\nmake: *** [sha256.o] Error 1\n",&testReport);
	gs_report_setAuthorize("test",&testReport);
	gs_report_setOrigin("admin",&testReport);
	gs_report_setType("INFO",&testReport);

   	db_insertReport(&testReport,conn);

   	stringToReturn = malloc(1000);
	request.method = DELETE;
	
	/* Valids */
	sprintf(request.url, "/api/debug?origin=%s&hash=%s", "admin", testReport.authorize);
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(204, status, "Request failed to return no content")

	/* Invalids */

	sprintf(request.url, "/api/debug?origin=%s&hash=%s", "admin", testReport.authorize);
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(404, status, "Request failed to return not found status")

	sprintf(request.url, "/api/debug");
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Request failed to err when no params present")	

	sprintf(request.url, "/api/debug?origin=%s", testReport.origin);
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Request failed to err when given only origin")

	sprintf(request.url, "/api/debug?hash=%s", testReport.authorize);
	status = report_controller(&request, stringToReturn, 1000);
	EXPECTED(400, status, "Request failed to err when given only hash")


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