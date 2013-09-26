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


void db_getScopeById(long id, struct gs_scope * gss, MYSQL * conn){
   MYSQL_RES * result;
   MYSQL_ROW row; 
   char query[64];

   /*Zero the scope structure */
   gs_scope_ZeroStruct(gss);

   bzero(query,64);
   sprintf(query, GS_SCOPE_GET_BY_ID, id);

   if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }

   result = mysql_use_result(conn);
   row = mysql_fetch_row(result);
   if(row == NULL){
      mysql_free_result(result);
      return;    
   }

   /* Make sure id is integer */
   gs_scope_setId(atol(row[0]), gss);
   gs_scope_setDesc(row[1], gss);

   mysql_free_result(result);
}

/* We assume the calling party has used the page size to set the size of gsc */
int db_getComments(int page, struct gs_comment * gsc, MYSQL * conn){
   MYSQL_RES * result;
   MYSQL_ROW row; 
   int i;
   char query[90];

   bzero(query,90);
   sprintf(query, GS_COMMENT_GET_ALL, page);

   if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return 0;
   }

   i=0;
   result = mysql_use_result(conn);
   while( (row=mysql_fetch_row(result)) != NULL ){
      /* Initialize */
      gs_comment_ZeroStruct(&gsc[i]);

      gs_comment_setId( atol(row[0]), &gsc[i]);
      gs_comment_setContent( row[1], &gsc[i]);
      gs_comment_setScopeId( atol(row[2]), &gsc[i]);
      gs_comment_setCreatedTime( row[3], &gsc[i]);
      i++;
   }
   mysql_free_result(result);  
   return i;
}

void db_getCommentById(long id, struct gs_comment * gsc, MYSQL * conn){
   MYSQL_RES * result;
   MYSQL_ROW row; 
   char query[80]; /* 72 for query, 8 for padding and null char*/

   /*Zero the scope structure */
   gs_comment_ZeroStruct(gsc);

   bzero(query,80);
   sprintf(query, GS_COMMENT_GET_BY_ID, id);

   if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }

   result = mysql_use_result(conn);
   row = mysql_fetch_row(result);
   if(row == NULL){
      mysql_free_result(result);
      return;    
   }

   /* Make sure id is integer */
   gs_comment_setId( atol(row[0]), gsc);
   gs_comment_setContent( row[1], gsc);
   gs_comment_setScopeId( atol(row[2]), gsc);
   gs_comment_setCreatedTime( row[3], gsc);

   mysql_free_result(result);  
}

#ifndef DB_INSERT_COMMENT_QUERY_SIZE
   #define DB_INSERT_COMMENT_QUERY_SIZE 58+140+10
#endif
void db_insertComment(struct gs_comment * gsc, MYSQL * conn){
   MYSQL_RES * result;
   MYSQL_ROW row; 
   long affected;
   char query[DB_INSERT_COMMENT_QUERY_SIZE]; /* Query, content, id, some extra padding*/


   if(gsc->scopeId == GS_SCOPE_INVALID_ID)
      return; /* Return if scope is invalid that we can tell*/

   bzero(query,DB_INSERT_COMMENT_QUERY_SIZE);
   sprintf(query, GS_COMMENT_INSERT, gsc->content, gsc->scopeId);

    if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }

   affected = mysql_insert_id(conn);
   if( affected == 0){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }

   /* Set the id of the comment to be what it is now  */
   gsc->id = affected;

   /* Now we could either compute the time stamp or ask the db for it. */
   bzero(query,DB_INSERT_COMMENT_QUERY_SIZE);
   sprintf(query,GS_COMMENT_GET_BY_ID, affected);
   
   /* Fresh Start and we want to return to the user EXACTLY what's in the db */
   gs_comment_ZeroStruct(gsc);

   if(0 != mysql_query(conn, query) ){
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }

   result = mysql_use_result(conn);
   row = mysql_fetch_row(result);
   if(row == NULL){
      mysql_free_result(result);
      return;    
   }

   /* Fill er up */
   gs_comment_setId( atol(row[0]), gsc);
   gs_comment_setContent( row[1], gsc);
   gs_comment_setScopeId( atol(row[2]), gsc);
   gs_comment_setCreatedTime( row[3], gsc);

   mysql_free_result(result);
   
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