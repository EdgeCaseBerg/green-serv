#include "config.h"
#include "db.h"
#include "models/scope.h"
#include "helpers/json.h"
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
   	
   	char json[512];
   	bzero(json,512);

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

	/*Clean Up database connection*/
	mysql_close(conn);
	mysql_library_end();
}
