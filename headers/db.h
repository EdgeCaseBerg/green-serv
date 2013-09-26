#ifndef __DB_H__
	#define __DB_H__
	
	#include <mysql/mysql.h>

	#include <stdio.h>
	#include <stdlib.h>
	#include "scope.h"
	#include "comment.h"
	#include "config.h"
	#include <string.h>
	
	/* How many of an entity per page*/
	#define RESULTS_PER_PAGE 20
	#define TOSTR(x) #x
	#define STRINGIFY(x) TOSTR(x)
	#define GS_INVALID_ID (-1)

	/*Returns a connection to the mySQL database.*/
	MYSQL * _getMySQLConnection();

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

	/* Just a test function */
	int testDB();
#endif