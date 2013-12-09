#include "models/comment.h"
#include "db.h"
#include "config.h"
#include "helpers/json.h"
#include <unistd.h>

#define JSON_LENGTH 1000
int main(){
	MYSQL * conn;
	struct gs_comment testComment;
	struct gs_comment * commentPage;
	char json[JSON_LENGTH];
	int numComments;
	int i;
	bzero(json,JSON_LENGTH);

	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setCommentType("COMMENT", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);
	   
   	db_insertComment(&testComment,conn);

   	bzero(json,JSON_LENGTH);

   	gs_commentToNJSON(testComment,json,JSON_LENGTH);
   	printf("%s\n", json);

   	/* test getting comment by id */
	db_getCommentById(testComment.id,&testComment,conn);
	gs_commentToNJSON(testComment,json,JSON_LENGTH);
	printf("%s\n", json);

	commentPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_comment));
	if(commentPage != NULL){
	  	numComments = db_getComments(0, CAMPAIGN_ID,commentPage, conn);
	  	for(i=0; i < numComments; ++i){
		 	bzero(json,JSON_LENGTH);
		 	gs_commentToNJSON(commentPage[i],json,JSON_LENGTH);
		 	printf("%s\n", json);	  
	  	}

	  	free(commentPage);
   	}else{
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for comment page");
   	}

   	mysql_close(conn);
	mysql_library_end();

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

}
#undef JSON_LENGTH