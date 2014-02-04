#include "db.h"
#ifndef DATABASE_LOGGING
	#define DATABASE_LOGGING 0
#endif
#if(DATABASE_LOGGING != 1)
	#ifdef DATABASE_LOGGING
		#undef DATABASE_LOGGING
	#endif
	#define DATABASE_LOGGING 0
#endif
#define LOGDB if(DATABASE_LOGGING == 1) fprintf(stderr, "%s\n", query);
#define LOGDBTRANS(status) if(DATABASE_LOGGING == 1) fprintf(stderr, "Transaction has been %s\n", status);

/* _shared_campaign_id is declared in config.h and is a global
 * readonly variable to be used for scoping purposes
*/

MYSQL * _getMySQLConnection(){
	MYSQL *conn;
	char *server = HOST;
	char *user = USERNAME;
	char *password = PASSWORD; /* set me first */
	char *database = DATABASE;
	#ifndef THREADED_DB
	/* If the connections are threaded this will be called in the main */
	mysql_library_init(0, NULL, NULL);
	#endif

	conn = mysql_init(NULL);
	return mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
}

/* Word of warning. 
 * If you begin a transaction, you may NOT insert, read, and delete the same
 * row in the database within the same transaction. Why? Because you'll dead
 * -lock the whole shebang for 50 seconds or whatever the lock time out is. 
 * Why does this occur? Because we've locked the row when we select it by it's
 * ID, or if it's queried for at all, so we can't delete it after that.
 */
void db_start_transaction(MYSQL * conn){
	mysql_autocommit(conn, 0);
	LOGDBTRANS("started");
}

void db_abort_transaction(MYSQL * conn){
	mysql_rollback(conn);
	LOGDBTRANS("aborted");
}

void db_end_transaction(MYSQL * conn){
	mysql_commit(conn);
	mysql_autocommit(conn, 1);
	LOGDBTRANS("ended");
}

void db_getScopeById(long id, struct gs_scope * gss, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	char query[64];

	/*Zero the scope structure */
	gs_scope_ZeroStruct(gss);

	bzero(query,sizeof query);
	sprintf(query, GS_SCOPE_GET_BY_ID, id);
	LOGDB
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

void db_insertScope(struct gs_scope * gss, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	char query[ (sizeof GS_SCOPE_INSERT) + sizeof(long) + 10 ]; /* Query, content, id, some extra padding*/

	bzero(query,sizeof query);
	sprintf(query, GS_SCOPE_INSERT, gss->description );
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		gss->id = GS_SCOPE_INVALID_ID;
		return;
	}

	affected = mysql_insert_id(conn);
	if( affected == 0){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}

	/* Set the id of the comment to be what it is now  */
	gss->id = affected;

	
	/* Fresh Start and we want to return to the user EXACTLY what's in the db */
	bzero(query,sizeof query);
	sprintf(query,GS_SCOPE_GET_BY_ID, affected);
	gs_scope_ZeroStruct(gss);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		gss->id = GS_SCOPE_INVALID_ID;
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}

	result = mysql_use_result(conn);
	row = mysql_fetch_row(result);
	if(row == NULL){
		gss->id = GS_SCOPE_INVALID_ID;
		mysql_free_result(result);
		return;    
	}

	/* Fill er up */
	gs_scope_setId( atol(row[0]), gss);
	gs_scope_setDesc(row[1], gss);
	mysql_free_result(result);  
}

/* We assume the calling party has used the page size to set the size of gsc */
int db_getComments(int page, long scopeId, struct gs_comment * gsc, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	int i;
	int limit;
	char query[sizeof GS_COMMENT_GET_ALL];

	bzero(query,sizeof query);
	/* In order to return the correct paginated results we have the following
	 * strategy: Retrieve the results per page, this is +1 more than we will
	 * be sending to the client. This is to make the 'nextUrl' without having
	 * to check the database for the total number of comments.
	 * To avoid losing that last telltale comment (since we discard it and it's
	 * not sent to the client) the limit offset has to be reduced by 1 if we're
	 * asking for more than one page.  
	 * For more detail and some math, see 6aa7d80
	*/
	limit = page*RESULTS_PER_PAGE;
	limit = limit > 0 ? limit-(page) : limit;
	sprintf(query, GS_COMMENT_GET_ALL, scopeId, limit);

	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", query);
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/* Initialize */
		gs_comment_ZeroStruct(&gsc[i]);
		gs_comment_setId( atol(row[0]), &gsc[i]);
		gs_comment_setPinId(row[1] == NULL ? -1 : atol(row[1]), &gsc[i]);
		gs_comment_setContent( row[2], &gsc[i]);
		gs_comment_setScopeId( atol(row[3]), &gsc[i]);
		gs_comment_setCreatedTime( row[4], &gsc[i]);
		gs_comment_setCommentType(row[5], &gsc[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

int db_getCommentsByType(int page, long scopeId, struct gs_comment * gsc, char * cType, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	int i;
	int limit;
	char query[sizeof GS_COMMENT_GET_BY_TYPE + GS_COMMENT_TYPE_LENGTH];

	bzero(query,sizeof query);
	/* For reasoning on the limit calculations:
	 * see 6aa7d80
	*/
	limit = page*RESULTS_PER_PAGE;
	limit = limit > 0 ? limit-(page) : limit;
	sprintf(query, GS_COMMENT_GET_BY_TYPE, scopeId, cType,limit);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", query);
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/* Initialize */
		gs_comment_ZeroStruct(&gsc[i]);

		gs_comment_setId( atol(row[0]), &gsc[i]);
		gs_comment_setPinId(row[1] == NULL ? -1 : atol(row[1]), &gsc[i]);
		gs_comment_setContent( row[2], &gsc[i]);
		gs_comment_setScopeId( atol(row[3]), &gsc[i]);
		gs_comment_setCreatedTime( row[4], &gsc[i]);
		gs_comment_setCommentType( row[5], &gsc[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

void db_getCommentById(long id, struct gs_comment * gsc, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	char query[sizeof GS_COMMENT_GET_BY_ID]; /* 72 for query, 8 for padding and null char*/

	/*Zero the scope structure */
	gs_comment_ZeroStruct(gsc);

	bzero(query,sizeof query);
	sprintf(query, GS_COMMENT_GET_BY_ID, id);
	LOGDB
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
	gs_comment_setPinId(row[1] == NULL ? -1 : atol(row[1]), gsc);
	gs_comment_setContent( row[2], gsc);
	gs_comment_setScopeId( atol(row[3]), gsc);
	gs_comment_setCreatedTime( row[4], gsc);
	gs_comment_setCommentType(row[5], gsc);

	mysql_free_result(result);  
}

#ifndef DB_INSERT_COMMENT_QUERY_SIZE
	#define DB_INSERT_COMMENT_QUERY_SIZE 58 + GS_COMMENT_MAX_LENGTH + 1 +10 + sizeof GS_COMMENT_INSERT
#endif
void db_insertComment(struct gs_comment * gsc, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	char query[DB_INSERT_COMMENT_QUERY_SIZE]; /* Query, content, id, some extra padding*/


	if(gsc->scopeId == GS_SCOPE_INVALID_ID)
		return; /* Return if scope is invalid that we can tell*/

	bzero(query,sizeof query);
	sprintf(query, GS_COMMENT_INSERT, gsc->content, gsc->scopeId, gsc->pinId,gsc->cType);
	LOGDB
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
	bzero(query,sizeof query);
	sprintf(query,GS_COMMENT_GET_BY_ID, affected);
   
	/* Fresh Start and we want to return to the user EXACTLY what's in the db */
	gs_comment_ZeroStruct(gsc);
	LOGDB
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
	gs_comment_setPinId(row[1] == NULL ? -1 : atol(row[1]), gsc);
	gs_comment_setContent( row[2], gsc);
	gs_comment_setScopeId( atol(row[3]), gsc);
	gs_comment_setCreatedTime( row[4], gsc);
	gs_comment_setCommentType( row[5], gsc);

	mysql_free_result(result);
   
}

int db_getMarkers(int page, long scopeId, struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	Decimal longitude;
	int i;
	int limit;
	char query[sizeof GS_MARKER_GET_ALL];
	bzero(query,sizeof query);
	
	limit = page*MARKER_LIMIT;
	limit = limit > 0 ? limit-(page) : limit;

	sprintf(query, GS_MARKER_GET_ALL, scopeId, limit);
	LOGDB
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
		latitude = createDecimalFromString(row[4]);
		gs_marker_setLatitude(latitude,&gsm[i]);
		longitude = createDecimalFromString(row[5]);
		gs_marker_setLongitude(longitude,&gsm[i]);
		gs_marker_setAddressed(atoi(row[6]), &gsm[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

#ifndef DB_INSERT_MARKER_QUERY_SIZE
	#define DB_INSERT_MARKER_QUERY_SIZE 128
#endif
void db_insertMarker(struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	char query[DB_INSERT_MARKER_QUERY_SIZE]; /* Query, content, id, some extra padding*/

	if(gsm->scopeId == GS_SCOPE_INVALID_ID)
		return; /* Return if scope is invalid that we can tell*/

	bzero(query,sizeof query);
	sprintf(query, GS_MARKER_INSERT, gsm->commentId, gsm->scopeId, gsm->latitude,  gsm->longitude,  gsm->addressed);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}

	affected = mysql_insert_id(conn);
	if( affected == 0){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}

	/* Set the id of the marker to be what it is now  */
	gsm->id = affected;

	/* Now we could either compute the time stamp or ask the db for it. */
	bzero(query,sizeof query);
	sprintf(query,GS_MARKER_GET_BY_ID, affected);
	LOGDB	
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
	gsm->latitude = createDecimalFromString(row[4]);
	gsm->longitude = createDecimalFromString(row[5]);
	gs_marker_setAddressed(atoi(row[6]), gsm);
	

	mysql_free_result(result);
   
}


void db_getMarkerById(long id, struct gs_marker * gsm, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	char query[128]; /* 128 For safety. */

	/*Zero the scope structure */
	gs_marker_ZeroStruct(gsm);

	bzero(query,sizeof query);
	sprintf(query, GS_MARKER_GET_BY_ID, id);
	LOGDB
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
	gsm->latitude = createDecimalFromString(row[4]);
	gsm->longitude = createDecimalFromString(row[5]);
	gs_marker_setAddressed(atoi(row[6]), gsm);
	


	mysql_free_result(result);  
}



#ifndef DB_INSERT_HEATMAP_QUERY_SIZE
	#define DB_INSERT_HEATMAP_QUERY_SIZE 198/* 198 for safety */
#endif
void db_insertHeatmap(struct gs_heatmap * gsh, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	int updated;
	char query[DB_INSERT_HEATMAP_QUERY_SIZE]; /* Query, content, id, some extra padding*/

	if(gsh->scopeId == GS_SCOPE_INVALID_ID)
		return; /* Return if scope is invalid that we can tell*/

	/* Check for possible merges */
	bzero(query,sizeof query);
	sprintf(query, GS_HEATMAP_FIND_MATCH, gsh->scopeId, gsh->latitude, gsh->longitude);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}


	result = mysql_use_result(conn);
	row = mysql_fetch_row(result);
	bzero(query, sizeof query);
	updated = 0;
	if(row == NULL){
		mysql_free_result(result);
		/* This means we can insert a new one */
		sprintf(query, GS_HEATMAP_INSERT, gsh->scopeId, gsh->intensity, gsh->latitude, gsh->longitude);
	}else{
		/* Time to merge! */
		updated = 1;
		gsh->id = atol(row[0]);
		sprintf(query, GS_HEATMAP_UPDATE_BY_ID, (gsh->intensity + atol(row[1])) , atol(row[0]));	
		mysql_free_result(result);
	}

	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}

	if(updated == 0){
		/* Only if we inserted is the mysql_insert_id going to return the id 
		 * a bit of an kick is that mysql_insert_id doesn't return the id of
		 * the last UPDATED row because it assumes you already know the id.
		 * the documentation does say that it should work for update, kinda:
		 * "Returns the value generated for an AUTO_INCREMENT column by the 
		    previous INSERT or UPDATE statement. Use this function after you 
		    have performed an INSERT statement into a table that contains an 
		    AUTO_INCREMENT field, or have used INSERT or UPDATE to set a column 
		    value with LAST_INSERT_ID(expr)." -- http://dev.mysql.com/doc/refman/5.0/en/mysql-insert-id.html
		 * so our update statement apparently doesn't fit the requirement.
		*/
		affected = mysql_insert_id(conn);
		if( affected == 0){
			fprintf(stderr, "%s\n", mysql_error(conn));
			return;
		}

		/* Set the id of the comment to be what it is now  */
		gsh->id = affected;	
	}
	
	/* Now we could either compute the time stamp or ask the db for it. */
	bzero(query,sizeof query);
	sprintf(query,GS_HEATMAP_GET_BY_ID, gsh->id);

	/* Fresh Start and we want to return to the user EXACTLY what's in the db */
	gs_heatmap_ZeroStruct(gsh);
	LOGDB
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
	gs_heatmap_setId( atol(row[0]), gsh);
	gs_heatmap_setIntensity( atol(row[1]), gsh);
	gs_heatmap_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), gsh);
	gs_heatmap_setCreatedTime( row[3], gsh);
	gsh->latitude = createDecimalFromString(row[4]);
	gsh->longitude = createDecimalFromString(row[5]);
	

	mysql_free_result(result);
   
}

#ifndef HEATMAP_PAGE_QUERY_SIZE
	#define HEATMAP_PAGE_QUERY_SIZE 600 /* Super safe estimate (could probably just be 388 or so)*/
#endif
int db_getHeatmap(int page, long scopeId, long precision, long * max, Decimal lowerLatBound, Decimal upperLatBound, Decimal lowerLonBound, Decimal upperLonBound, struct gs_heatmap * gsh, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	Decimal longitude;
	int i;
	int limit;
	char query[HEATMAP_PAGE_QUERY_SIZE];

	limit = page*HEATMAP_RESULTS_PER_PAGE;
	limit = limit > 0 ? limit-(page) : limit;
	bzero(query,sizeof query);
	sprintf(query, 	GS_HEATMAP_GET_ALL, 
				   	precision, 	/* Latitude precision */
				   	precision, 	/* Longitude precision */
				   	scopeId,  	/* Scope */
				   	lowerLatBound, /* Latitude lower bound  */
					upperLatBound, /* Latitude upper bound  */
					lowerLonBound, /* Longitude lower bound */
					upperLonBound, /* Longitude upper bound */
					precision, /* Truncation of Grouping criteria */
					precision, /* Truncation of Grouping criteria */
				   	limit);

	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	(*max) = 0;
	while( (row=mysql_fetch_row(result)) != NULL ){
		/* Initialize */
		gs_heatmap_ZeroStruct(&gsh[i]);

		gs_heatmap_setId( page, &gsh[i]); /* Just setting the page as the id becuase these points are merged grouped heatmap points*/
		gs_heatmap_setIntensity( atol(row[0]), &gsh[i]);
		if(gsh[i].intensity > (*max))
			(*max) = gsh[i].intensity;
		gs_heatmap_setScopeId( scopeId, &gsh[i]);
		gs_heatmap_setCreatedTime( row[1] == NULL ? "" : row[1], &gsh[i]);
		latitude = createDecimalFromString(row[2]);
		gs_heatmap_setLatitude(latitude,&gsh[i]);
		longitude = createDecimalFromString(row[3]);
		gs_heatmap_setLongitude(longitude,&gsh[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

#ifndef DB_INSERT_REPORT_QUERY_SIZE
	#define DB_INSERT_REPORT_QUERY_SIZE 96 + GS_REPORT_TYPE_MAX_LENGTH + GS_REPORT_MAX_LENGTH + (SHA_LENGTH+1)*2 +4/* 96 for Query, 65*2 for hashes, +4 for safety */
#endif
void db_insertReport(struct gs_report * gsr, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	long affected;
	char query[DB_INSERT_REPORT_QUERY_SIZE]; 

	if(gsr->scopeId == GS_SCOPE_INVALID_ID)
		return; /* Return if scope is invalid that we can tell*/

	bzero(query,sizeof query);
	sprintf(query, GS_REPORT_INSERT, gsr->content, gsr->scopeId, gsr->origin, gsr->authorize, gsr->trace, gsr->rType);
	LOGDB
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
	gsr->id = affected;

	/* Now we could either compute the time stamp or ask the db for it. */
	bzero(query,sizeof query);
	sprintf(query,GS_REPORT_GET_BY_AUTH, gsr->authorize);

	/* Fresh Start and we want to return to the user EXACTLY what's in the db */
	gs_report_ZeroStruct(gsr);
	LOGDB
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
	gs_report_setId( atol(row[0]), gsr);
	gs_report_setContent( row[1], gsr);
	gs_report_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), gsr);
	strncpy(gsr->origin,row[3], SHA_LENGTH);
	strncpy(gsr->authorize, row[4], SHA_LENGTH);
	gs_report_setCreatedTime( row[5], gsr);
	gs_report_setStackTrace( row[6], gsr);
	gs_report_setType(row[7], gsr);
	

	mysql_free_result(result);
   
}

void db_getReportByAuth(char * auth, struct gs_report * gsr, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	char query[99+65+40+GS_REPORT_TYPE_MAX_LENGTH]; /* 99 for query, 65 for auth hash, 4 for safety*/

	gs_report_ZeroStruct(gsr);
	bzero(query,sizeof query);
	
	sprintf(query, GS_REPORT_GET_BY_AUTH, auth);
	LOGDB
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

	gs_report_setId( atol(row[0]), gsr);
	gs_report_setContent( row[1], gsr);
	gs_report_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), gsr);
	strncpy(gsr->origin,row[3], SHA_LENGTH);
	strncpy(gsr->authorize, row[4], SHA_LENGTH);
	gs_report_setCreatedTime( row[5], gsr);
	gs_report_setStackTrace( row[6], gsr);
	gs_report_setType(row[7], gsr);

	mysql_free_result(result);  
}

int db_deleteReport(struct gs_report * gsr, MYSQL * conn){
	char query[99+(64*2)+5+100]; /* 61 for query, 64*2+1 for hashes, 4 for safety*/

	bzero(query,sizeof query);
	sprintf(query, GS_REPORT_DELETE, gsr->origin,gsr->authorize);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	return mysql_affected_rows(conn);  
}

int db_deleteComment( long id, MYSQL * conn){
	char query[128]; 

	bzero(query, sizeof query);
	sprintf(query, GS_COMMENT_DELETE, id);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	return mysql_affected_rows(conn);  
}


int db_addressMarker(long id, int addressed, MYSQL * conn){
	char query[sizeof GS_MARKER_ADDRESS]; 

	bzero(query, sizeof query);
	sprintf(query, GS_MARKER_ADDRESS, addressed,id);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	return mysql_affected_rows(conn);  
}

int db_deleteMarker(long id, MYSQL * conn){
	char query[128]; 

	bzero(query, sizeof query);
	sprintf(query, GS_MARKER_DELETE, id);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	return mysql_affected_rows(conn);  
}

#ifndef REPORT_PAGE_QUERY_SIZE
	#define REPORT_PAGE_QUERY_SIZE 300 
#endif
int db_getReports(int page,char * since, long scopeId,  struct gs_report * gsr, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	int i;
	int limit;
	char query[REPORT_PAGE_QUERY_SIZE];
	bzero(query,sizeof query);

	limit = page*RESULTS_PER_PAGE;
	limit = limit > 0 ? limit-(page) : limit;
	sprintf(query,GS_REPORT_GET_ALL,scopeId, since, limit);

	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/* Initialize */
		gs_report_ZeroStruct(&gsr[i]);

		gs_report_setId( atol(row[0]), &gsr[i]);
		gs_report_setContent( row[1], &gsr[i]);
		gs_report_setScopeId( row[2] == NULL ? GS_SCOPE_INVALID_ID : atol(row[2]), &gsr[i]);
		strncpy(gsr[i].origin,row[3], SHA_LENGTH);
		strncpy(gsr[i].authorize, row[4], SHA_LENGTH);
		gs_report_setCreatedTime( row[5], &gsr[i]);
		gs_report_setStackTrace( row[6], &gsr[i]);
		gs_report_setType(row[7], &gsr[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}


/* This is the 'get all' queries functions for the hybrid 
 * gsm and gsc have to be of the correct size for this function
 * to work, they need to have MARKER_LIMIT sizes
*/
int db_getMarkerComments(int page, long scopeId, struct gs_marker * gsm, struct gs_comment * gsc, MYSQL * conn){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	int limit;
	Decimal longitude;
	int i;
	char query[sizeof GS_MARKER_COMMENT_GET_ALL];

	bzero(query,sizeof query);
	limit = page*MARKER_LIMIT;
	limit = limit > 0 ? limit-(page) : limit;
	sprintf(query, GS_MARKER_COMMENT_GET_ALL, scopeId, limit);

	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/*  Fields:
			pin_id, comment_id, content, comment_type, latitude, longitude, addressed
		*/
		gs_marker_ZeroStruct(&gsm[i]);
		gs_comment_ZeroStruct(&gsc[i]);

		gs_marker_setId( atol(row[0]), &gsm[i]);
		gs_marker_setCommentId( atol(row[1]), &gsm[i]);
		gs_comment_setId(atol(row[1]), &gsc[i]);
		gs_comment_setContent(row[2], &gsc[i]);
		gs_marker_setScopeId( scopeId, &gsm[i]);
		gs_comment_setScopeId(scopeId, &gsc[i]);
		gs_comment_setCommentType(row[3], &gsc[i]);
		latitude = createDecimalFromString(row[4]);
		gs_marker_setLatitude(latitude,&gsm[i]);
		longitude = createDecimalFromString(row[5]);
		gs_marker_setLongitude(longitude,&gsm[i]);
		gs_marker_setAddressed(atoi(row[6]), &gsm[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

/* Worker function to handle filtering by a single coordinate axis.
 * when calling, all parameters should be properly initialized.
*/
static int db_getMarkerCommentsCoordinate(int page, long scopeId, struct gs_marker * gsm, struct gs_comment * gsc, MYSQL * conn,Decimal * center, Decimal * offset, const char * queryString){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	Decimal longitude;
	Decimal lowDec;
	Decimal upDec;
	int limit;
	int i;
	char query[strlen(queryString)];
	char lower[DecimalWidth];
	char upper[DecimalWidth];

	bzero(query,sizeof query);
	bzero(lower,sizeof lower);
	bzero(upper,sizeof upper);

	lowDec = createDecimalFromString("0.0");
	upDec = createDecimalFromString( "0.0");

	limit = page*MARKER_LIMIT;
	limit = limit > 0 ? limit-(page) : limit;

	/* calculate the bounds via the center and offset */
	add_decimals(center, offset, &upDec); 	
    subtract_decimals(center, offset, &lowDec); 
    formatDecimal(lowDec, lower);
    formatDecimal(upDec, upper);

	sprintf(query, queryString, scopeId, lower, upper ,limit);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/*  Fields:
			pin_id, comment_id, content, comment_type, latitude, longitude, addressed
		*/
		gs_marker_ZeroStruct(&gsm[i]);
		gs_comment_ZeroStruct(&gsc[i]);

		gs_marker_setId( atol(row[0]), &gsm[i]);
		gs_marker_setCommentId( atol(row[1]), &gsm[i]);
		gs_comment_setId(atol(row[1]), &gsc[i]);
		gs_comment_setContent(row[2], &gsc[i]);
		gs_marker_setScopeId( scopeId, &gsm[i]);
		gs_comment_setScopeId(scopeId, &gsc[i]);
		gs_comment_setCommentType(row[3], &gsc[i]);
		latitude = createDecimalFromString(row[4]);
		gs_marker_setLatitude(latitude,&gsm[i]);
		longitude = createDecimalFromString(row[5]);
		gs_marker_setLongitude(longitude,&gsm[i]);
		gs_marker_setAddressed(atoi(row[6]), &gsm[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}

int db_getMarkerCommentsLatitude(int page, long scopeId, struct gs_marker * gsm, struct gs_comment * gsc, MYSQL * conn,Decimal * center, Decimal * latOffset){
	return db_getMarkerCommentsCoordinate(page,scopeId,gsm,gsc,conn,center,latOffset,GS_MARKER_COMMENT_GET_BY_LATITUDE);
}

int db_getMarkerCommentsLongitude(int page, long scopeId, struct gs_marker * gsm, struct gs_comment * gsc, MYSQL * conn,Decimal * center, Decimal * lonOffset){
	return db_getMarkerCommentsCoordinate(page,scopeId,gsm,gsc,conn,center,lonOffset,GS_MARKER_COMMENT_GET_BY_LONGITUDE);
}

int db_getMarkerCommentsFullFilter(int page, long scopeId, struct gs_marker * gsm, struct gs_comment * gsc, MYSQL * conn,Decimal * latCenter, Decimal * latOffset, Decimal * lonCenter, Decimal * lonOffset){
	MYSQL_RES * result;
	MYSQL_ROW row; 
	Decimal latitude;
	Decimal longitude;
	Decimal lowDec;
	Decimal upDec;
	int i;
	int limit;
	char query[sizeof GS_MARKER_COMMENT_GET_BY_BOTH];
	char latLower[DecimalWidth];
	char latUpper[DecimalWidth];
	char lonLower[DecimalWidth];
	char lonUpper[DecimalWidth];

	bzero(query,sizeof query);
	bzero(latLower,sizeof latLower);
	bzero(latUpper,sizeof latUpper);
	bzero(lonLower,sizeof lonLower);
	bzero(lonUpper,sizeof lonUpper);

	/* Initialize */
	lowDec = createDecimalFromString("0.0");
	upDec = createDecimalFromString( "0.0");

	limit = page*MARKER_LIMIT;
	limit = limit > 0 ? limit-(page) : limit;

	/* calculate the bounds via the center and offset */
	add_decimals(latCenter, latOffset, &upDec); 	
    subtract_decimals(latCenter, latOffset, &lowDec); 
    formatDecimal(lowDec, latLower);
    formatDecimal(upDec, latUpper);

	add_decimals(lonCenter, lonOffset, &upDec); 	
    subtract_decimals(lonCenter, lonOffset, &lowDec); 
    formatDecimal(lowDec, lonLower);
    formatDecimal(upDec, lonUpper);    
   
	sprintf(query, GS_MARKER_COMMENT_GET_BY_BOTH, scopeId, latLower, latUpper, lonLower, lonUpper ,limit);
	LOGDB
	if(0 != mysql_query(conn, query) ){
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}

	i=0;
	result = mysql_use_result(conn);
	while( (row=mysql_fetch_row(result)) != NULL ){
		/*  Fields:
			pin_id, comment_id, content, comment_type, latitude, longitude, addressed
		*/
		gs_marker_ZeroStruct(&gsm[i]);
		gs_comment_ZeroStruct(&gsc[i]);

		gs_marker_setId( atol(row[0]), &gsm[i]);
		gs_marker_setCommentId( atol(row[1]), &gsm[i]);
		gs_comment_setId(atol(row[1]), &gsc[i]);
		gs_comment_setContent(row[2], &gsc[i]);
		gs_marker_setScopeId( scopeId, &gsm[i]);
		gs_comment_setScopeId(scopeId, &gsc[i]);
		gs_comment_setCommentType(row[3], &gsc[i]);
		latitude = createDecimalFromString(row[4]);
		gs_marker_setLatitude(latitude,&gsm[i]);
		longitude = createDecimalFromString(row[5]);
		gs_marker_setLongitude(longitude,&gsm[i]);
		gs_marker_setAddressed(atoi(row[6]), &gsm[i]);
		i++;
	}
	mysql_free_result(result);  
	return i;
}