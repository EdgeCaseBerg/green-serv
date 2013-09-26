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

   /*Clean Up database connection*/
   mysql_close(conn);
   mysql_library_end();
}
