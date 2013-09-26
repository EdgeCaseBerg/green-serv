GreenServ
========================================================================

Author:
- Ethan J. Eldridge


------------------------------------------------------------------------


GreenServ is a simple RAW request/response API server to service the
GreenUp Vermont event. Currently, the clients use a google app engine
database which has results in many a headache. This pet project creates 
a fast reliable. Consistent. And Speedy server program.

Setup:

Example apache virtual hosts configuration:
------------------------------------------------------------------------

	<VirtualHost *:80>
        Servername green.xenonapps.dev
        DocumentRoot /green-serv/
        ErrorLog /green-serv/error.log
        CustomLog /green-serv/access.log combined
        ProxyPass / http://localhost:10110/
        ProxyPassReverse / http://localhost:10110/
     </VirtualHost>

Example Configuration File:
Place the configuration file into the headers directory before building.
------------------------------------------------------------------------

	#ifndef __CONFIG_H__
		#define __CONFIG_H__
	
		#define PORT 10110
		#define HOST "green.xenonapps.dev"
		#define DATABASE "green_xenon"
		#define PASSWORD "pylons"
		#define USERNAME "tassadar"
        #define CAMPAIGN_ID 0L
        #define CAMPAIGN_DESC "8CharMax"
	
	#endif


SQL to create database structure:
------------------------------------------------------------------------

    CREATE DATABASE the_database;
    
    CREATE USER 'user'@'localhost' IDENTIFIED BY 'cleartextpassword';
    GRANT ALL ON the_database.* TO 'user'@'localhost';
    FLUSH PRIVILEGES;
    
    USE the_database;
    
    CREATE TABLE scope (
    	id INT(12) NOT NULL auto_increment PRIMARY KEY,
    	description CHAR(8) NOT NULL, -- Yeah just 8 characters
    	INDEX(`id`)
    ) ENGINE InnoDB;
    
    
    #Seed The Database with initial ancestor
    INSERT INTO scope (description) VALUES ('GREEN_UP');
    
    #Cache table for fast pagination
    CREATE TABLE cacheComments (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        content VARCHAR(140) NOT NULL,
        scope_id INT(12) NOT NULL, -- this is an ancestor style query
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
    ) ENGINE Memory;


    CREATE TABLE comments (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        content VARCHAR(140),
        scope_id INT(12) NOT NULL, -- this is an ancestor style query
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        INDEX (`scope_id`),
        CONSTRAINT FOREIGN KEY (`scope_id`) REFERENCES `scope` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
    ) ENGINE InnoDB;

    #High speed reading
    CREATE TABLE cacheMarkers (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        comment_id INT(12) NULL,
        scope_id INT(12), -- this is an ancestor style query
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        latitude DECIMAL(10, 8) NOT NULL,
        longitude DECIMAL(11, 8) NOT NULL,
        INDEX (`id`)
    ) ENGINE Memory;


    CREATE TABLE markers (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        comment_id INT(12) NULL,
        scope_id INT(12), -- this is an ancestor style query
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        latitude DECIMAL(10, 8) NOT NULL,
        longitude DECIMAL(11, 8) NOT NULL,
        INDEX (`id`),
        CONSTRAINT FOREIGN KEY (`comment_id`) REFERENCES `comments` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
        CONSTRAINT FOREIGN KEY (`scope_id`) REFERENCES `scope` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
    ) ENGINE InnoDB;


    CREATE TABLE heatmap (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        scope_id INT(12), -- this is an ancestor style query
        intensity INT(12), -- this is seconds worked
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- for dampening
        latitude DECIMAL(10, 8) NOT NULL,
        longitude DECIMAL(11, 8) NOT NULL,
        INDEX(`scope_id`),
        CONSTRAINT FOREIGN KEY (`scope_id`) REFERENCES `scope` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
    ) ENGINE InnoDB;

    CREATE TABLE report (
        id INT(12) NOT NULL auto_increment PRIMARY KEY,
        content TEXT, -- stores 64Kb.
        scope_id INT(12), -- this is an ancestor style query
        origin CHAR(64) NOT NULL, -- sha256 hash
        authorize CHAR(64) NOT NULL, -- sha256 hash
        created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        INDEX(`scope_id`),
        CONSTRAINT FOREIGN KEY (`scope_id`) REFERENCES `scope` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
    ) ENGINE InnoDB;


Example Compilation and Verification:
------------------------------------------------------------------------

	make
	md5sum a.out
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./a.out

