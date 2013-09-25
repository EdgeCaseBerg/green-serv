#ifndef __DB_H__
	#define __DB_H__
	
	#include <mysql/mysql.h>

	#include <stdio.h>
	#include <stdlib.h>
	#include "scope.h"
	#include "config.h"
	#include <string.h>


	/*Returns a connection to the mySQL database.*/
	MYSQL * _getMySQLConnection();

	/* Given a struct and an id, return a structure populate from db*/
	void db_getScopeById(long id, struct gs_scope * gss);

	/* Just a test function */
	int testDB();
#endif