#ifndef __DB_H__
	#define __DB_H__
	
	#include <mysql/mysql.h>

	#include <stdio.h>
	#include <stdlib.h>
	#include "scope.h"
	#include "comment.h"
	#include "marker.h"
	#include "config.h"
	#include <string.h>
	
	/* How many of an entity per page*/
	#define RESULTS_PER_PAGE 20
	#define TOSTR(x) #x
	#define STRINGIFY(x) TOSTR(x)
	#define GS_INVALID_ID (-1)

	/*Returns a connection to the mySQL database.*/
	MYSQL * _getMySQLConnection();

	/* Helper function to create a decimal with arbitrary precision */
	void createDecimalFromString(Decimal * dec, char * str);

	/* Given a struct and an id, return a structure populate from db*/
	void db_getScopeById(long id, struct gs_scope * gss, MYSQL * conn);

	/* Insert a comment struct and return the database's version of it*/
	void db_insertComment(struct gs_comment * gsc, MYSQL * conn);

	/* Get a single comment by it's id */
	void db_getCommentById(long id, struct gs_comment * gsc, MYSQL * conn);

	/* Get comments by page. The size of each page is determined by RESULTS_PER_PAGE in db.h 
	 * returns the number of comments returned, make sure to use this value
	 * If unsure about what scopeId to pass, send the invalid Scope and the default configured 
	 * scope will be used instead.
	*/
	int db_getComments(int page, long scopeId, struct gs_comment * gsc, MYSQL * conn);

	/*Same warning for getComments, you need to make sure there is enough space.
	 *
	*/
	int db_getMarkers(int page, long scopeId, struct gs_marker * gsm, MYSQL * conn);

	/* Insert a single marker into the database */
	void db_insertMarker(struct gs_marker * gsm, MYSQL * conn);

#endif