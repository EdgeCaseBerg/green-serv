#ifndef __NET_H__
	#define __NET_H__

	#define METHOD_LENGTH 7
	#define HASH_TABLE_CAPACITY 15
	#define MAX_URL_LENGTH 100
	#define FIRSTLINEBUFFSIZE 256
	#define GET 2
	#define POST 4
	#define PUT 8
	#define DELETE 16
	#define UNKNOWN_METHOD 32
	#define NUMTHREADS 1/*00*/
	#define DETACHED_THREADS
	#undef DETACHED_THREADS
	struct http_request{
		int method;
		char url[MAX_URL_LENGTH]; /* The request URL to determine the controller */
		int contentLength; /* The length of the content coming in */
		char * data; /* This must be malloced for data */
	};
	/* Simple struct to contain data to be sent to worker threads */
	struct threadData{
	    char msg[1024];
	    int clientfd;
	};

	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <fcntl.h>
	#include <time.h>
	#include <pthread.h>
	#include "config.h"
	#include "network/router.h"
	#include "helpers/strmap.h"
	#include "controllers/heartbeat.h"




	/*Create an HTTP Json response
	*/
	int createResponse(char * content, char * buff, int status);

	/*Create Socket for communications
 	*returns file descriptor or -1 on err
	*/
	int createSocket();

	/* Sets up a socket for use as a server socket on the designated port
	 * returns -1 on errors, 0 otherwise
	 * fd is the file descriptor of the socket to setup
	 * sockserv is a pointer to a sockaddr_in struct.
	 * port is the port to bind the address to
	*/
	int setupSockAndBind(int fd, struct sockaddr_in * sockserv, int port );

	/*Parses a String looking for an http request. Fills up a struct
	 *with the url, method, and any data along with it. (Data is malloced)
	 *and must be free. Check contentLength field of struct to determine
	 *if you should call free on it or not.
	*/
	int parseRequest(struct http_request * requestToFill, char * requestStr);

	/*Function responsible for doing work in a thread. Passed as an argument
	 *to test_network function.
	*/
	void* doNetWork(struct threadData* td);

	/* This function is unlikely to live beyond testing things */
	int test_network(char * buffer, int bufferLength, void*( *func )(void*) );

	/* Parse a url and return the number of parameters begotten from it */
	int parseURL(char * url, int urlLength, StrMap * table);

#endif 