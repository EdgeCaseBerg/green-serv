#include "db.h"
#include "config.h"
#include "models/comment.h"
#include "models/marker.h"
#include "helpers/json.h"

int main(){
	MYSQL * conn;
	struct gs_comment testComment;
	struct gs_marker testMarker;
   	struct gs_marker * markerPage;
   	Decimal latitude;
   	Decimal longitude;
   	char json[512];
   	int numMarkers;
   	int i;
	bzero(json,512);

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

   	/* Setup referenced comment */
   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);
	   
   	db_insertComment(&testComment,conn);

   	createDecimalFromString(&latitude, "-44.050");
	createDecimalFromString(&longitude, "-44.70");

   
	gs_marker_ZeroStruct(&testMarker);

	gs_marker_setCommentId(testComment.id, &testMarker);
	gs_marker_setScopeId(CAMPAIGN_ID, &testMarker);
	gs_marker_setLongitude(longitude, &testMarker);
	gs_marker_setLatitude(latitude, &testMarker);

	db_insertMarker(&testMarker, conn);
	gs_markerToJSON(testMarker, json);
	printf("%s\n", json);	

	db_getMarkerById(testMarker.id, &testMarker, conn);
	gs_markerToJSON(testMarker, json);
	printf("%s\n", json);	

	markerPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_marker));
	if(markerPage != NULL){

		numMarkers = db_getMarkers(0, CAMPAIGN_ID, markerPage, conn);
		for(i=0; i < numMarkers; ++i){
			bzero(json,512);
			gs_markerToJSON(markerPage[i], json);
			printf("%s\n", json);		
		}
		
		free(markerPage);
	}else{	
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for marker page");
	}

	mysql_close(conn);
	mysql_library_end();
	

}