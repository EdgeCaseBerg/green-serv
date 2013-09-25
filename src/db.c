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

int testDB(){
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