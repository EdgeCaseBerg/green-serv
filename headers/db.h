#ifndef __DB_H__
	#define __DB_H__
	
	#include <mysql/mysql.h>

	#include <stdio.h>
	#include <stdlib.h>

	/*Returns a connection to the mySQL database.*/
	MYSQL * getMySQLConnection();

	/* Just a test function */
	int testDB();
#endif