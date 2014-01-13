#ifndef __NET_H__
	#define __NET_H__

	#define METHOD_LENGTH 7
	
	#define FIRSTLINEBUFFSIZE 512
	#define NUMTHREADS 100
	#define DETACHED_THREADS
	#undef DETACHED_THREADS /* Using detacthed threads runs the risk of leaving fd's open */
	/* Simple struct to contain data to be sent to worker threads */
	struct threadData{
	    char msg[16384];
	    int clientfd;
	};

	
	#include <unistd.h>
	#include <signal.h>
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
	#include "controllers/comments.h"
	#include "controllers/heartbeat.h"
	#include "controllers/markers.h"
	#include "controllers/heatmaps.h"
	#include "controllers/reports.h"
	#include <sys/wait.h>
	

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

	int run_network(char * buffer, int bufferLength, void*( *func )(void*) );



#endif 