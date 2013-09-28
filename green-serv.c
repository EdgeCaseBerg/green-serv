#include "config.h"
#include "db.h"
#include "models/scope.h"
#include "json.h"
#include "flags.h"
#include "models/comment.h"
#include "models/marker.h"
#include "models/heatmap.h"
#include "models/report.h"
#include <string.h>


/* Check for flags. */
void parseArgs(int argc, const char * argv[], struct gs_scope * campaign, MYSQL * conn){
	int i;
	for(i=0; i < argc; ++i){
		if ( strncasecmp(argv[i], SCOPE_ID_SHORT, strlen(SCOPE_ID_SHORT)) == 0 || strncasecmp(argv[i], SCOPE_ID_LONG, strlen(SCOPE_ID_LONG)) == 0)
			if ( argc >= i+1 )
				db_getScopeById(atol(argv[++i]), campaign,conn);       

	}
}

int main(int argc, const char* argv[]) {
	MYSQL * conn;
   	struct gs_scope campaign;
   	struct gs_comment testComment;
   	struct gs_comment * commentPage;
   	struct gs_marker testMarker;
   	struct gs_marker * markerPage;
   	struct gs_heatmap testHeatmap;
   	struct gs_heatmap * heatmapPage;
   	struct gs_report testReport;
   	Decimal latitude;
   	Decimal longitude;
   	Decimal lowerBoundLat;
   	Decimal lowerBoundLon;
   	Decimal upperBoundLat;
   	Decimal upperBoundLon;
   	char json[512];
   	bzero(json,512);
   	int numComments;
   	int numMarkers;
   	int numHeatmap;
   	int i;

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

   	/* Setup Campaign for all querying. */
   	db_getScopeById(CAMPAIGN_ID, &campaign, conn);
   	parseArgs(argc,argv,&campaign,conn);

	gs_scopeToJSON(campaign,json);
	
   	printf("%s\n", json);

   	/* Test comment here */
   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setScopeId(campaign.id, &testComment);
	   
   	db_insertComment(&testComment,conn);

   	bzero(json,512);

   	gs_commentToJSON(testComment,json);
   	printf("%s\n", json);

   	/* test getting comment by id */
	db_getCommentById(testComment.id,&testComment,conn);
	gs_commentToJSON(testComment,json);
	printf("%s\n", json);

	commentPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_comment));
	if(commentPage != NULL){

	  	numComments = db_getComments(0, campaign.id,commentPage, conn);
	  	for(i=0; i < numComments; ++i){
		 	bzero(json,512);
		 	gs_commentToJSON(commentPage[i],json);
		 	printf("%s\n", json);	  
	  	}

	  	free(commentPage);
   	}else{
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for comment page");
   	}

	/* Test markers */
	createDecimalFromString(&latitude, "-44.50");
	createDecimal(-44, 70, &longitude);
   
	gs_marker_ZeroStruct(&testMarker);

	gs_marker_setCommentId(testComment.id, &testMarker);
	gs_marker_setScopeId(campaign.id, &testMarker);
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

		numMarkers = db_getMarkers(0, campaign.id, markerPage, conn);
		for(i=0; i < numMarkers; ++i){
			bzero(json,512);
			gs_markerToJSON(markerPage[i], json);
			printf("%s\n", json);		
		}
		
		free(markerPage);
	}else{	
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for marker page");
	}

	bzero(json,512);
	gs_heatmap_ZeroStruct(&testHeatmap);
	gs_heatmapToJSON(testHeatmap, json);
	printf("%s\n", json);

	gs_heatmap_setIntensity( 2, &testHeatmap);
	gs_heatmap_setScopeId( campaign.id, &testHeatmap);
	createDecimalFromString(&testHeatmap.latitude,"-44.781");
	createDecimalFromString(&testHeatmap.longitude,"70.11");
	
	db_insertHeatmap(&testHeatmap, conn);

	gs_heatmapToJSON(testHeatmap, json);
	printf("%s\n", json);

	createDecimalFromString(&lowerBoundLat, "-50.0");
	createDecimalFromString(&upperBoundLat, "-43.78");
	createDecimalFromString(&lowerBoundLon, "69.9");
	createDecimalFromString(&upperBoundLon, "71.78");
	heatmapPage = malloc(HEATMAP_RESULTS_PER_PAGE* sizeof(struct gs_heatmap));
	if(heatmapPage != NULL){
		numHeatmap = db_getHeatmap(	
					/* page 	*/ 		0,
					/* scope 	*/ 	campaign.id, 
					/* precision*/	3, 
					/* Low Lat 	*/ 	lowerBoundLat,
					/* Up Lat  	*/	upperBoundLat,
					/* Low Lon 	*/	lowerBoundLon,
					/* Up Lat 	*/ 	upperBoundLon,
					/* array 	*/ 	heatmapPage,
					/* mysql con*/	conn
					);
		for(i=0; i < numHeatmap; ++i){
			bzero(json,512);
			gs_heatmapToJSON(heatmapPage[i], json);
			printf("%s\n", json);
		}
		free(heatmapPage);
	}else{
		fprintf(stderr, "%s\n", "Could not allocate enough memory for heatmap page");
	}
	
	gs_report_ZeroStruct(&testReport);



	/*Clean Up database connection*/
	mysql_close(conn);
	mysql_library_end();
}
