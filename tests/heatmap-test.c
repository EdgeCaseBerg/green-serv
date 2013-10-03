#include "config.h"
#include "helpers/json.h"
#include "helpers/decimal.h"
#include "models/heatmap.h"
#include "db.h"

#define HEATMAP_JSON_LENGTH 10
int main(){
	MYSQL * conn;
	Decimal lowerBoundLat;
   	Decimal lowerBoundLon;
   	Decimal upperBoundLat;
   	Decimal upperBoundLon;
   	struct gs_heatmap testHeatmap;
   	struct gs_heatmap * heatmapPage;
   	char json[HEATMAP_JSON_LENGTH];
   	int numHeatmap;
   	int i;
   	bzero(json,HEATMAP_JSON_LENGTH);

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

	gs_heatmap_ZeroStruct(&testHeatmap);
	gs_heatmapNToJSON(testHeatmap, json,HEATMAP_JSON_LENGTH);
	printf("%s\n", json);

	gs_heatmap_setIntensity( 2, &testHeatmap);
	gs_heatmap_setScopeId( CAMPAIGN_ID, &testHeatmap);
	createDecimalFromString(&testHeatmap.latitude,"-44.0781");
	createDecimalFromString(&testHeatmap.longitude,"70.011");
	
	db_insertHeatmap(&testHeatmap, conn);

	gs_heatmapNToJSON(testHeatmap, json,HEATMAP_JSON_LENGTH);
	printf("%s\n", json);

	createDecimalFromString(&lowerBoundLat, "-50.0");
	createDecimalFromString(&upperBoundLat, "-43.78");
	createDecimalFromString(&lowerBoundLon, "69.9");
	createDecimalFromString(&upperBoundLon, "71.78");
	heatmapPage = malloc(HEATMAP_RESULTS_PER_PAGE* sizeof(struct gs_heatmap));
	if(heatmapPage != NULL){
		numHeatmap = db_getHeatmap(	
					/* page 	*/ 		0,
					/* scope 	*/ 	CAMPAIGN_ID, 
					/* precision*/	8, 
					/* Low Lat 	*/ 	lowerBoundLat,
					/* Up Lat  	*/	upperBoundLat,
					/* Low Lon 	*/	lowerBoundLon,
					/* Up Lat 	*/ 	upperBoundLon,
					/* array 	*/ 	heatmapPage,
					/* mysql con*/	conn
					);
		for(i=0; i < numHeatmap; ++i){
			bzero(json,HEATMAP_JSON_LENGTH);
			gs_heatmapNToJSON(heatmapPage[i], json,HEATMAP_JSON_LENGTH);
			printf("%s\n", json);
		}
		free(heatmapPage);
	}else{
		fprintf(stderr, "%s\n", "Could not allocate enough memory for heatmap page");
	}

	mysql_close(conn);
	mysql_library_end();
}