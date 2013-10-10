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
	#include "models/report.h"
	#include "helpers/decimal.h"
	#include "config.h"
	#include <string.h>
	
	/* How many of an entity per page
	 * +1 more than actually returned because we can test for a next page
	 * that way.
	*/
	#define RESULTS_PER_PAGE (21)
	#define RESULTS_RETURNED (RESULTS_PER_PAGE-1)
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
	#define GS_MARKER_INSERT "INSERT INTO markers (comment_id, scope_id, latitude, longitude) VALUES (%ld, %ld, %ld.%08lu, %ld.%08lu);"

	#define GS_HEATMAP_GET_ALL "SELECT SUM(intensity), TIMESTAMP(AVG(created_time)) ,TRUNCATE(latitude,%ld), TRUNCATE(longitude,%ld) FROM heatmap WHERE scope_id = %ld AND latitude BETWEEN %ld.%08lu AND %ld.%08lu AND longitude BETWEEN %ld.%08lu AND %ld.%08lu GROUP BY latitude ORDER BY created_time DESC LIMIT %d, " STRINGIFY(HEATMAP_RESULTS_PER_PAGE) ";"
	#define GS_HEATMAP_GET_BY_ID "SELECT id, intensity, scope_id, created_time, latitude, longitude FROM heatmap WHERE id = %ld;"
	#define GS_HEATMAP_INSERT "INSERT INTO heatmap (scope_id, intensity, latitude, longitude) VALUES (%ld, %ld, %ld.%08lu, %ld.%08lu);"
	#define GS_HEATMAP_FIND_MATCH "SELECT id, intensity FROM heatmap WHERE scope_id = %ld AND latitude = %ld.%08lu AND longitude = %ld.%08lu;"
	#define GS_HEATMAP_UPDATE_BY_ID "UPDATE heatmap SET intensity = %ld WHERE id = %ld;"

	#define GS_REPORT_GET_ALL "SELECT id, content, scope_id, origin, authorize, created_time FROM report WHERE scope_id = %ld ORDER BY created_time DESC LIMIT %d, " STRINGIFY(RESULTS_PER_PAGE) ";" 
	#define GS_REPORT_GET_BY_AUTH "SELECT id, content, scope_id, origin, authorize, created_time FROM report WHERE authorize = \"%s\";"
	#define GS_REPORT_INSERT "INSERT INTO report (content, scope_id, origin, authorize) VALUES (\"%s\", %ld, \"%s\", \"%s\");"
	#define GS_REPORT_DELETE "DELETE FROM report WHERE origin =\"%s\" AND authorize=\"%s\";"


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

	/* Returns a page from the heatmap. Some important notes:  
	 * - The id's for the heatmap structs returned will be the same as the page! 
	 *   this is because averaging an id in the database makes 0 sense. And 
	 *   since the heatmaps from this function are merged and grouped and bucket
	 *   sorted, it makes no sense to return an id becuase they are an 
	 *   aggregation of points, not the points themself.
	 * page: The page to retrieve
	 * scopeId: The scope of the points to recieve (multilayer support of heatmaps
	 *    		by multi scopes client side if they want to implement it)
	 * precision: Truncates after the decimal to 'precision' places. Example: 
	 *			  44.781 with precision 2 becomes 44.78 This is NOT a round. 
	 * lowerLatBound
	 * A couple warnings about this function. Similarally to the other pagination
	 * functions, you need to make sure that the structure array passed in that
	 * will be populated is large enough for at least HEATMAP_RESULTS_PER_PAGE 
	 * many entities. Note that the heatmap has its own constant for page size
	 * this is because you need more data to created a good heatmap then you 
	 * might need for comments or markers. 
	*/
	int db_getHeatmap(int page, long scopeId, long precision, Decimal lowerLatBound, Decimal upperLatBound, Decimal lowerLonBound, Decimal upperLonBound, struct gs_heatmap * gsh, MYSQL * conn);

	void db_insertReport(struct gs_report * gsr, MYSQL * conn);

	void db_getReportByAuth(char * auth, struct gs_report * gsr, MYSQL * conn);

	int db_deleteReport(struct gs_report * gsr, MYSQL * conn);

	int db_getReports(int page, long scopeId, struct gs_report * gsr, MYSQL * conn);

#endif