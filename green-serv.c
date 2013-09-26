#include "config.h"
#include "db.h"
#include "scope.h"
#include "json.h"
#include "flags.h"
#include "comment.h"
#include <string.h>

/* Check for flags. */
void parseArgs(int argc, const char * argv[], struct gs_scope * campaign){
   int i;
   for(i=0; i < argc; ++i){
      if ( strncasecmp(argv[i], SCOPE_ID_SHORT, strlen(SCOPE_ID_SHORT)) == 0 || strncasecmp(argv[i], SCOPE_ID_LONG, strlen(SCOPE_ID_LONG)) == 0)
         if ( argc >= i+1 )
            db_getScopeById(atol(argv[++i]), campaign);           

   }
}

int main(int argc, const char* argv[]) {
   struct gs_scope campaign;
   struct gs_comment testComment;
   char json[512];
   bzero(json,512);

   /* Setup Campaign for all querying. */
   db_getScopeById(CAMPAIGN_ID, &campaign);
   parseArgs(argc,argv,&campaign);

   gsScopeToJSON(campaign,json);
    
   printf("%s\n", json);

   printf("%s\n", "uhm could you pleae work no?");
   /* Test comment here */
   gs_comment_ZeroStruct(&testComment);
   gs_comment_setContent("Test Comment", &testComment);
   gs_comment_setScopeId(campaign.id, &testComment);
   
   db_insertComment(&testComment);

   bzero(json,512);

   gsCommentToJSON(testComment,json);
   printf("%s\n", json);

}
