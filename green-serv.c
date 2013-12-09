/*Defined GREENSERV in order to initalize global in this file*/
#define GREENSERV 1
#include "config.h"
#include "db.h"
#include "models/scope.h"
#include "helpers/json.h"
#include "flags.h"
#include "models/comment.h"
#include "models/marker.h"
#include "models/heatmap.h"
#include "models/report.h"
#include "network/net.h"
#include <unistd.h>
#include <string.h>

#ifndef BOOT_LOGGING
    #define BOOT_LOGGING 0
#endif
#if(BOOT_LOGGING != 1)
    #ifdef BOOT_LOGGING
        #undef BOOT_LOGGING
    #endif
    #define BOOT_LOGGING 0
#endif
/*pre must be a string declared like "sting" not a variable*/
#define BOOT_LOG_STR(pre,s) if(BOOT_LOGGING == 1) fprintf(stderr, pre "%s\n", (s));

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
	/*You must initialize the library on the main thread. 
	 *the only time threaded_db should be undef-ed is if you're
	 *unit testing and don't want a threaded environment for some
	 *weird reason.
	*/
	#ifdef THREADED_DB
	BOOT_LOG_STR("Initializing MySQL Library.","");
	mysql_library_init(0, NULL, NULL);
	#endif

	conn = _getMySQLConnection();
	if(!conn){
		fprintf(stderr, "%s\n", "Could not connect to mySQL");
		return 1;
	}

	/* Setup Campaign for all querying. */
	db_getScopeById(CAMPAIGN_ID, &campaign, conn);
	parseArgs(argc,argv,&campaign,conn);
	mysql_close(conn);
	
	/* Set global campaign id for this Server instance */
	_shared_campaign_id = campaign.id;
	(void)_shared_campaign_id; /*http://stackoverflow.com/a/394568/1808164*/

	char buff[1024];
	bzero(buff,1024);
	/*What a silly cast we have to make...*/
	BOOT_LOG_STR("Running Network Interface:", "");
	run_network(buff,1024,(void*(*)(void*))&doNetWork);
	BOOT_LOG_STR("Finished Network Interface.","");
	/*Clean Up database connection*/
	mysql_library_end();

	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef BOOT_LOG_STR