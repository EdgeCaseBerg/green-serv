#include "config.h"
#include "db.h"
#include "models/scope.h"
#include <unistd.h>


int main(){
	MYSQL * conn;
	struct gs_scope campaign;

	/* Seed Database with Scope from Config file. */

	conn = _getMySQLConnection();
	if(!conn){
		fprintf(stderr, "%s\n", "Could not connect to mySQL");

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return 1;
	}

	gs_scope_setId(CAMPAIGN_ID, &campaign );
	gs_scope_setDesc("GREEN_UP" , &campaign);

	db_start_transaction(conn);
	db_insertScope(&campaign, conn);

	if(campaign.id == CAMPAIGN_ID)
		fprintf(stdout, "Created Campaign Scope with requested campaign ID of %ld\n", CAMPAIGN_ID);
	else if(campaign.id == GS_SCOPE_INVALID_ID)
		fprintf(stderr, "There was a problem attempting to create the scope.\n" );
	else
		fprintf(stdout, "Could not created campaign with requested ID %ld, created with ID of: %ld\n", CAMPAIGN_ID, campaign.id);

	
   	db_end_transaction(conn);
	mysql_close(conn);
	mysql_library_end();
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}
	