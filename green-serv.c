#include "config.h"
#include "db.h"
#include "scope.h"
#include "json.h"
#include "flags.h"
#include "comment.h"
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
   struct gs_comment * commentsPage;
   char json[512];
   bzero(json,512);
   int numComments;
   int i;

   conn = _getMySQLConnection();
   if(!conn){
      fprintf(stderr, "%s\n", "Could not connect to mySQL");
      return 1;
   }

   /* Setup Campaign for all querying. */
   db_getScopeById(CAMPAIGN_ID, &campaign, conn);
   parseArgs(argc,argv,&campaign,conn);

   gsScopeToJSON(campaign,json);
    
   printf("%s\n", json);

   /* Test comment here */
   gs_comment_ZeroStruct(&testComment);
   gs_comment_setContent("Test Comment", &testComment);
   gs_comment_setScopeId(campaign.id, &testComment);
   
   db_insertComment(&testComment,conn);

   bzero(json,512);

   gsCommentToJSON(testComment,json);
   printf("%s\n", json);

   /* test getting comment by id */
   db_getCommentById(testComment.id,&testComment,conn);
   gsCommentToJSON(testComment,json);
   printf("%s\n", json);

   commentsPage = malloc(RESULTS_PER_PAGE * sizeof(struct gs_comment));
   if(commentsPage != NULL){

      numComments = db_getComments(0, commentsPage, conn);
      for(i=0; i < numComments; ++i){
         gsCommentToJSON(commentsPage[i],json);
         printf("%s\n", json);      
      }

      free(commentsPage);
   }else{
      fprintf(stderr, "%s\n", "Could not allocate enough memory for comment page");
   }


   /*Clean Up database connection*/
   mysql_close(conn);
   mysql_library_end();
}
