
#include "db.h"

int main() {
   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;

   conn = getMySQLConnection();
   
   if (!conn) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }

   /* send SQL query */
   if (mysql_query(conn, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }
   res = mysql_use_result(conn);
   /* output table name */
   printf("MySQL Tables in mysql database:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);
   /* close connection */
   mysql_free_result(res);
   mysql_close(conn);

   /*Do this or leak. */
   mysql_library_end();
   return 0;
}