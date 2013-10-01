#include "models/comment.h"
#include "db.h"
#include "config.h"
#include "helpers/json.h"

int main(){
	MYSQL * conn;
	struct gs_comment testComment;
	struct gs_comment * commentPage;
	char json[512];
	int numComments;
	int i;
	bzero(json,512);

	conn = _getMySQLConnection();
   	if(!conn){
	  	fprintf(stderr, "%s\n", "Could not connect to mySQL");
	  	return 1;
   	}

   	gs_comment_ZeroStruct(&testComment);
   	gs_comment_setContent("Test Comment", &testComment);
   	gs_comment_setScopeId(CAMPAIGN_ID, &testComment);
	   
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

	  	numComments = db_getComments(0, CAMPAIGN_ID,commentPage, conn);
	  	for(i=0; i < numComments; ++i){
		 	bzero(json,512);
		 	gs_commentToJSON(commentPage[i],json);
		 	printf("%s\n", json);	  
	  	}

	  	free(commentPage);
   	}else{
	  	fprintf(stderr, "%s\n", "Could not allocate enough memory for comment page");
   	}

   	mysql_close(conn);
	mysql_library_end();


}