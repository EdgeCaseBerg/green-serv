#include "db.h"


MYSQL * _getMySQLConnection(){
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


void db_getScopeById(long id, struct gs_scope * gss){
   MYSQL *conn;
   MYSQL_RES * result;
   MYSQL_ROW row; 
   char query[64];


   /*Zero the scope structure */
   gs_scope_ZeroStruct(gss);

   conn = _getMySQLConnection();
   if(!conn)
      return; /* Callee must check for zero-ed gss*/

   bzero(query,64);
   sprintf(query, GS_SCOPE_GET_BY_ID, id);

   if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      mysql_close(conn);
      mysql_library_end();
      return;
   }

   result = mysql_use_result(conn);
   row = mysql_fetch_row(result);
   if(row == NULL){
      mysql_free_result(result);
      mysql_close(conn);
      mysql_library_end();
      return;    
   }
      

   /* Make sure id is integer */
   gs_scope_setId(atol(row[0]), gss);
   gs_scope_setDesc(row[1], gss);

   mysql_free_result(result);
   mysql_close(conn);

   /*Do this or leak. */
   mysql_library_end();

}


int testDB(){
	MYSQL *conn;
   	MYSQL_RES *res;
   	MYSQL_ROW row;

   	conn = _getMySQLConnection();
	   
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