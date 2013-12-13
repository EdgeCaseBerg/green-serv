#include "db.h"
#include "config.h"
#include "models/comment.h"
#include "models/marker.h"
#include "helpers/json.h"
#include <unistd.h>

#define JSON_LENGTH 512
int main(){
	MYSQL * conn;
	struct gs_comment testComment;
	struct gs_marker testMarker;
   	struct gs_marker * markerPage;
   	Decimal latitude;
   	Decimal longitude;
   	char json[JSON_LENGTH];
   	int numMarkers;
   	int i;
	bzero(json,JSON_LENGTH);

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}
   	db_start_transaction(conn);
   	
   	/* Setup referenced comment */
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
	gs_markerNToJSON(testMarker, json, JSON_LENGTH);
	printf("%s\n", json);	

	db_getMarkerById(testMarker.id, &testMarker, conn);
	gs_markerNToJSON(testMarker, json, JSON_LENGTH);
	printf("%s\n", json);	

	markerPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_marker));
	if(markerPage != NULL){

		numMarkers = db_getMarkers(0, CAMPAIGN_ID, markerPage, conn);
		for(i=0; i < numMarkers; ++i){
			bzero(json,JSON_LENGTH);
			gs_markerNToJSON(markerPage[i], json, JSON_LENGTH);
			printf("%s\n", json);		
		}
		
		free(markerPage);
	}else{	
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for marker page");
	}

	db_abort_transaction(conn);
   	db_end_transaction(conn);
	mysql_close(conn);
	mysql_library_end();
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

}
#undef JSON_LENGTH