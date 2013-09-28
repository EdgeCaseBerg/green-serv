#ifndef __DB_H__
	#define __DB_H__
	
	#include <mysql/mysql.h>

	#include "flags.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include "models/scope.h"
	#include "models/comment.h"
	#include "models/marker.h"
	#include "models/heatmap.h"
	#include "config.h"
	#include <string.h>
	
	/* How many of an entity per page*/
	#define RESULTS_PER_PAGE 20
	/* Gets it's own results because it's more likely we'll want more
	 * heatmap data than something like comments
	*/
	#define HEATMAP_RESULTS_PER_PAGE 50
	#define TOSTR(x) #x
	#define STRINGIFY(x) TOSTR(x)

	/* SQL Queries for entities */	
	#define GS_SCOPE_GET_ALL "SELECT id, description FROM scope LIMIT %d;"
	#define GS_SCOPE_GET_BY_ID "SELECT id, description FROM scope WHERE id = %ld;"
	#define GS_SCOPE_INSERT "INSERT INTO scope (description) VALUES (\"%s\");"

	#define GS_COMMENT_GET_ALL "SELECT id, content, scope_id, created_time FROM comments WHERE scope_id = %ld ORDER BY created_time DESC LIMIT %d, " STRINGIFY(RESULTS_PER_PAGE) ";"
	#define GS_COMMENT_GET_BY_ID "SELECT id, content, scope_id, created_time FROM comments WHERE id = %ld;"
	#define GS_COMMENT_INSERT "INSERT INTO comments (content, scope_id) VALUES (\"%s\", %ld);"

	#define GS_MARKER_GET_ALL "SELECT id, comment_id, scope_id, created_time, latitude, longitude FROM markers WHERE scope_id = %ld ORDER BY created_time DESC LIMIT %d, " STRINGIFY(RESULTS_PER_PAGE) ";"
	#define GS_MARKER_GET_BY_ID "SELECT id, comment_id, scope_id, created_time, latitude, longitude FROM markers WHERE id = %ld;"
	#define GS_MARKER_INSERT "INSERT INTO markers (comment_id, scope_id, latitude, longitude) VALUES (%ld, %ld, %ld.%lu, %ld.%lu);"

	#define GS_HEATMAP_GET_ALL "SELECT intensity, latitude, longitude FROM heatmap WHERE scope_id = %ld ORDER BY created_time DESC LIMIT %d, " STRINGIFY(HEATMAP_RESULTS_PER_PAGE) ";"
	#define GS_HEATMAP_GET_BY_ID "SELECT id, intensity, scope_id, created_time, latitude, longitude FROM heatmap WHERE id = %ld;"
	#define GS_HEATMAP_INSERT "INSERT INTO heatmap (scope_id, intensity, latitude, longitude) VALUES (%ld, %ld, %ld.%lu, %ld.%lu);"

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

	/* Retrieve a single marker by it's id */
	void db_getMarkerById(long id, struct gs_marker * gsm, MYSQL * conn);

	/* Insert a single heatmap point into the database */
	void db_insertHeatmap(struct gs_heatmap * gsh, MYSQL * conn);

#endif