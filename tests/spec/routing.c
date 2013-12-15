#include "network/router.h"
#include <unistd.h>
#include <stdio.h>

static void expected(char * url, int expectedResponse){
	int response;
	response = determineController(url);
	if(response == expectedResponse)
		fprintf(stdout, "." );
	else{
		fprintf(stdout, "F" );
		fprintf(stderr, "ROUTING: Expected %d Got %d \n", expectedResponse, response);
	}
	
}

int main(){
	expected("",		HEARTBEAT_CONTROLLER);
	expected("/",		HEARTBEAT_CONTROLLER);
	expected("/api",	HEARTBEAT_CONTROLLER);
	expected("/api/",	HEARTBEAT_CONTROLLER);
	expected("/api/comments",	COMMENTS_CONTROLLER);
	expected("/api/comments/",	COMMENTS_CONTROLLER);
	expected("/api/comments?type=needs",	COMMENTS_CONTROLLER);
	expected("/api/heatmap",	HEATMAP_CONTROLLER);
	expected("/api/heatmap/",	HEATMAP_CONTROLLER);
	expected("/api/heatmap?latDegrees=23.45&latOffset=2.0&lonDegrees=40.3&lonOffset=5.12",	HEATMAP_CONTROLLER);
	expected("/api/pins",	MARKER_CONTROLLER);
	expected("/api/pins/",	MARKER_CONTROLLER);
	expected("/api/pins?id=32424j23k4j2kldsafasdf",	MARKER_CONTROLLER);
	expected("/api/debug",	REPORT_CONTROLLER);
	expected("/api/debug/",	REPORT_CONTROLLER);
	expected("/api/debug/?origin=6f3d78c8ca1d63645015d6fa2d975902348d585f954efd0e8ecca4f362c697d9&hash=aed60d05a1bd",	REPORT_CONTROLLER);
	expected("/api/de",		INVALID_CONTROLLER);
	expected("/api/asadssd",INVALID_CONTROLLER);
	expected("/ap",			INVALID_CONTROLLER);
	expected("/aasspi/asd",	INVALID_CONTROLLER);

	/* Be sure to flush before closing the lid on the i/o */
	fprintf(stdout, "\n" );
	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}