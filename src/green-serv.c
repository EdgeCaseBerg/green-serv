#include "db.h"
#include "scope.h"
#include "json.h"
#include <string.h>

int main() {
   struct gs_scope campaign;
   char json[100];
   bzero(json,100);

   campaign.id = 5;
   bzero(campaign.description,9);
   
   strncpy(campaign.description,"\b\t\n\\\f\r/greeen",8);

   gsScopeToJSON(campaign,json);
    

   printf("%s\n", json);

   testDB();
}
