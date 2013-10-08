#ifndef __NET_H__
	#define __NET_H__
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

	/* This function is unlikely to live beyond testing things */
	int test_network(char * buffer, int bufferLength, void*( *func )(void*) );

	#define METHOD_LENGTH 7
	#define MAX_URL_LENGTH 100
	#define FIRSTLINEBUFFSIZE 256
	#define GET 2
	#define POST 4
	#define PUT 8
	#define DELETE 16
	#define UNKNOWN_METHOD 32
	#define NUMTHREADS 100
	#define DETACHED_THREADS
	#undef DETACHED_THREADS
	struct http_request{
		int method;
		char url[MAX_URL_LENGTH]; /* The request URL to determine the controller */
		int contentLength; /* The length of the content coming in */
		char * data; /* This must be malloced for data */
	};

#endif 