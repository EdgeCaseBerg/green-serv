#include "network/router.h"
#include <unistd.h>

#include <stdio.h>

static void expected(char * url, int expectedResponse){
	int response;
	response = determineController(url);
	printf("Expected %d Got %d \n", expectedResponse, response);
}

int main(){
	expected("",HEARTBEAT_CONTROLLER);
	expected("/",HEARTBEAT_CONTROLLER);
	expected("/api",HEARTBEAT_CONTROLLER);
	expected("/api/",HEARTBEAT_CONTROLLER);

	expected("/api/comments",COMMENTS_CONTROLLER);
	expected("/api/comments/",COMMENTS_CONTROLLER);

	expected("/api/heatmap",HEATMAP_CONTROLLER);
	expected("/api/heatmap/",HEATMAP_CONTROLLER);

	expected("/api/pins",MARKER_CONTROLLER);
	expected("/api/pins/",MARKER_CONTROLLER);

	expected("/api/debug",REPORT_CONTROLLER);
	expected("/api/debug/",REPORT_CONTROLLER);

	expected("/api/de",INVALID_CONTROLLER);
	expected("/api/asadssd",INVALID_CONTROLLER);
	expected("/ap",INVALID_CONTROLLER);
	expected("/aasspi/asadssd",INVALID_CONTROLLER);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
}