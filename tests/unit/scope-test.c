#include "config.h"
#include "db.h"
#include "models/scope.h"
#include "helpers/json.h"
#include <unistd.h>


int main(){
	MYSQL * conn;
	struct gs_scope campaign;
	char json[512];
	char toSmall[8];
	bzero(json,512);
	bzero(toSmall,8);

	conn = _getMySQLConnection();
	if(!conn){
		fprintf(stderr, "%s\n", "Could not connect to mySQL");

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return 1;
	}

	/* Setup Campaign for all querying. */
	db_getScopeById(CAMPAIGN_ID, &campaign, conn);

	gs_scopeToJSON(campaign,json);
	printf("%s\n", json);

	/* Test the the safe version does not write to the output buffer */
	gs_scopeNToJSON(campaign,toSmall,8);
	printf("%s\n", toSmall);

	mysql_close(conn);
	mysql_library_end();   	

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

}