#include "config.h"
#include "db.h"
#include "models/scope.h"
#include "helpers/json.h"


int main(){
	MYSQL * conn;
   	struct gs_scope campaign;
   	char json[512];
   	bzero(json,512);

   	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

   	/* Setup Campaign for all querying. */
   	db_getScopeById(CAMPAIGN_ID, &campaign, conn);

	gs_scopeToJSON(campaign,json);
	
   	printf("%s\n", json);

   	mysql_close(conn);
	mysql_library_end();   	

}