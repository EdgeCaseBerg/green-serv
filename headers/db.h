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
	#define RESULTS_PER_PAGE (20)
	#define GS_INVALID_ID (-1)

	/*Returns a connection to the mySQL database.*/
	MYSQL * _getMySQLConnection();

	/* Given a struct and an id, return a structure populate from db*/
	void db_getScopeById(long id, struct gs_scope * gss, MYSQL * conn);

	/* Insert a comment struct and return the database's version of it*/
	void db_insertComment(struct gs_comment * gsc, MYSQL * conn);

	/* Get a single comment by it's id */
	void db_getCommentById(long id, struct gs_comment * gsc, MYSQL * conn);

	/* Just a test function */
	int testDB();
#endif