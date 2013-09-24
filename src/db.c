#include "db.h"
#include "config.h"

MYSQL * getMySQLConnection(){
   MYSQL *conn;
   char *server = HOST;
   char *user = USERNAME;
   char *password = PASSWORD; /* set me first */
   char *database = DATABASE;
   
   /* Call to init for multi-threading later on */
   mysql_library_init(0, NULL, NULL);

   conn = mysql_init(NULL);
   return mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
}
