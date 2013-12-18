#include "config.h"
#include <unistd.h>
#include "controllers/comments.h"

#define EXPECTED(expected, status, errmessage) \
	if( expected == status ) \
		fprintf(stdout, "." );\
	else{\
		fprintf(stdout, "F" );\
		fprintf(stderr, "DELETE COMMENT: Expected status of %d, recieved %d. %s (%s::%d)\n", expected,status, errmessage, __FILE__, __LINE__ );\
	}

int main(){
	char * stringToReturn;
	struct http_request request;
	int status;
	MYSQL * conn;
	struct gs_comment testComment;

	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	  	return 1;
   	}
   	

   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setCommentType("COMMENT", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);

   	db_insertComment(&testComment,conn);

   	stringToReturn = malloc(1000);
	request.method = DELETE;
	
	/* Valids */
	sprintf(request.url, "/api/comments?id=%ld", testComment.id);
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(204, status, "Request failed to return no content")

	/* Invalids */

	sprintf(request.url, "/api/comments?id=123234324");
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(404, status, "Request failed to return not found status")

	sprintf(request.url, "/api/comments");
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Request failed to err when no id present")	

	sprintf(request.url, "/api/comments?id=baulderdash");
	status = comment_controller(&request, stringToReturn, 1000);
	EXPECTED(422, status, "Request failed to err when given non-numeric id")




	mysql_close(conn);
	mysql_library_end();
		

	fflush(stdout);
	fflush(stderr);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

#undef EXPECTED