#include "helpers/json.h"
#include "models/report.h"
#include "db.h"
#include "config.h"

#define JSON_LENGTH GS_REPORT_MAX_LENGTH*4 /* EXTREMELY important here, you can get away with *3, but for absolutely safety
											* here and anywhere a json object is made for reports, there needs to be a LOT of
											* room or else you risk overwriting the stack.
											*/
int main(){
	MYSQL * conn;
	struct gs_report testReport;
   	struct gs_report * reportPage;
   	char json[JSON_LENGTH];
   	char auth[65];
   	int numReports;
   	int i;
	bzero(auth,65);
   	bzero(json,JSON_LENGTH);


   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

	gs_report_ZeroStruct(&testReport);

	gs_report_setScopeId(CAMPAIGN_ID, &testReport);
	gs_report_setContent("cc obj/sha256temp.o -o sha256.o -lcrypto\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 0 has invalid symbol index 10\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 1 has invalid symbol index 11\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 2 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 3 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 4 has invalid symbol index 10\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 5 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 6 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 7 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 8 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 9 has invalid symbol index 2\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 10 has invalid symbol index 11\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 11 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 12 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 13 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 14 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 15 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 16 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 17 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 18 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 19 has invalid symbol index 12\n/usr/bin/ld: /usr/lib/debug/usr/lib/x86_64-linux-gnu/crt1.o(.debug_info): relocation 20 has invalid symbol index 19\n/usr/lib/gcc/x86_64-linux-gnu/4.6/../../../x86_64-linux-gnu/crt1.o: In function `_start':\n(.text+0x20): undefined reference to `main'\ncollect2: ld returned 1 exit status\nmake: *** [sha256.o] Error 1\n",&testReport);
	gs_report_setAuthorize("test",&testReport);
	gs_report_setOrigin("admin",&testReport);
	db_insertReport(&testReport, conn);
	
	bzero(json,JSON_LENGTH);
	gs_reportNToJSON(testReport,json,JSON_LENGTH);
	printf("%s\n", json);
	
	reportPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_report));
	if(reportPage != NULL){
		numReports = db_getReports(0,"2000-00-00-00-00", CAMPAIGN_ID, reportPage, conn);;
		for(i=0; i < numReports; ++i){
			bzero(json,JSON_LENGTH);
			gs_reportNToJSON(reportPage[i], json,JSON_LENGTH);
			printf("%s\n", json);		
		}
		
		free(reportPage);
	}else{	
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for marker page");
	}

	strncpy(auth, testReport.authorize,64);
	db_getReportByAuth(auth, &testReport, conn);
	bzero(json,JSON_LENGTH);
	gs_reportNToJSON(testReport,json,JSON_LENGTH);
	printf("%s\n", json);

	printf("%d deleted\n", db_deleteReport(&testReport,  conn));

	mysql_close(conn);
	mysql_library_end();
}