#include "config.h"
#include "db.h"
#include "scope.h"
#include "json.h"
#include <string.h>

int main() {
   struct gs_scope campaign;
   char json[100];
   bzero(json,100);

   db_getScopeById(CAMPAIGN_ID, &campaign);


   gsScopeToJSON(campaign,json);
    

   printf("%s\n", json);

   testDB();
}
