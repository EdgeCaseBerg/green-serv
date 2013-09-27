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


void createDecimalFromString(Decimal * dec, char * str){
	long left;
	unsigned long right;

	if(str == NULL)
		return;

	if(sscanf(str, "%ld.%lu", &left, &right) == EOF)
		return;

	dec->left = left;
	dec->right = right;

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
int db_getComments(int page, long scopeId, struct gs_comment * gsc, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	int i;
	char query[110];

	bzero(query,110);
	sprintf(query, GS_COMMENT_GET_ALL, scopeId, page*RESULTS_PER_PAGE);

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

int db_getMarkers(int page, long scopeId, struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	Decimal longitude;
	int i;
	char query[140];

	bzero(query,140);
	sprintf(query, GS_MARKER_GET_ALL, scopeId, page*RESULTS_PER_PAGE);

	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/* Initialize */
		gs_marker_ZeroStruct(&gsm[i]);

		gs_marker_setId( atol(row[0]), &gsm[i]);
		gs_marker_setCommentId( atol(row[1]), &gsm[i]);
		gs_marker_setScopeId( row[2] == NULL ?  GS_SCOPE_INVALID_ID : atol(row[2]), &gsm[i]);
		gs_marker_setCreatedTime( row[3], &gsm[i]);
		createDecimalFromString(&latitude,row[4]);
		gs_marker_setLatitude(latitude,&gsm[i]);
		createDecimalFromString(&longitude,row[5]);
		gs_marker_setLongitude(longitude,&gsm[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

#ifndef DB_INSERT_MARKER_QUERY_SIZE
	#define DB_INSERT_MARKER_QUERY_SIZE 84 + 32 /* 84 for Query, 32 for safety */
#endif
void db_insertMarker(struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	char query[DB_INSERT_MARKER_QUERY_SIZE]; /* Query, content, id, some extra padding*/

	if(gsm->scopeId == GS_SCOPE_INVALID_ID)
		return; /* Return if scope is invalid that we can tell*/

	bzero(query,DB_INSERT_MARKER_QUERY_SIZE);
	sprintf(query, GS_MARKER_INSERT, gsm->commentId, gsm->scopeId, gsm->latitude.left, gsm->latitude.right, gsm->longitude.left, gsm->longitude.right);

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
	gsm->id = affected;

	/* Now we could either compute the time stamp or ask the db for it. */
	bzero(query,DB_INSERT_MARKER_QUERY_SIZE);
	sprintf(query,GS_MARKER_GET_BY_ID, affected);

	/* Fresh Start and we want to return to the user EXACTLY what's in the db */
	gs_marker_ZeroStruct(gsm);

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
	gs_marker_setId( atol(row[0]), gsm);
	gs_marker_setCommentId( atol(row[1]), gsm);
	gs_marker_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), gsm);
	gs_marker_setCreatedTime( row[3], gsm);
	createDecimalFromString(&gsm->latitude,row[4]);
	createDecimalFromString(&gsm->longitude,row[5]);
	

	mysql_free_result(result);
   
}


void db_getMarkerById(long id, struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	char query[95+5]; /* 95 for query, 5 for padding and null char*/

	/*Zero the scope structure */
	gs_marker_ZeroStruct(gsm);

	bzero(query,95+5);
	sprintf(query, GS_MARKER_GET_BY_ID, id);

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
	gs_marker_setId( atol(row[0]), gsm);
	gs_marker_setCommentId( row[1] == NULL ? GS_COMMENT_INVALID_ID :  atol(row[1]), gsm);
	gs_marker_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), gsm);
	gs_marker_setCreatedTime( row[3], gsm);
	createDecimalFromString(&gsm->latitude,row[4]);
	createDecimalFromString(&gsm->longitude,row[5]);
	


	mysql_free_result(result);  
}
