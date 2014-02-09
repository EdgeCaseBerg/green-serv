#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*Creates a tcp socket. Nothing special here...
*/
int _gh_create_tcp_socket() {
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Can't create TCP socket");
    return -1;
  }
  return sock;
}

/*Determines the ip of the host. 
 *If you know the ip already, you don't need to use this. But chances are
 *you don't know it, so here you go. A function to do it for ya.
*/
char *_gh_get_ip(char *host) {
    struct addrinfo hints, *res;
    struct in_addr addr;
    char * ip;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    printf("%d\n", EAI_SYSTEM);

    if ((err = getaddrinfo(host, NULL, &hints, &res)) != 0) {
        fprintf(stderr,"error %d\n", err);
        gai_strerror(err);
        return "PROBLEM";
    }

    addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;

    ip = inet_ntoa(addr);
    freeaddrinfo(res);

    return ip;
}

int main(){
	/*
to replicate. write program to send headers first in the middle of the \r\n\r\n 
and watch it break
	*/
	char * str_ip = "127.0.0.1";
	char * frame1 = "POST /api/comments HTTP/1.1\r\n\
Host: 199.195.248.180:31337\r\n\
Accept-Encoding: gzip, deflate\r\n\
Content-Type: application/json\r\n\
Accept-Language: en-us\r\n\
Connection: keep-alive\r\n\
Accept: */*\r\n\
Content-Length: 36\r\n\
User-Agent: GoGreen/1.0 CFNetwork/672.0.8 Darwin/13.0.0";
	char * frame2 = "\r\n\r\n{\"message\":\"post2\",\"type\":\"COMMENT\"}";
	int port = 80;


	struct sockaddr_in *remote;
  	int sock;
  	int tmpres;
  	char buf[BUFSIZ+1];  /*BUFSIZ comes from stdio.h*/
    unsigned int sent;
 
  	sock = _gh_create_tcp_socket();
  	
  	/* fprintf(stderr, "IP is %s\n", ip); */
  	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  	remote->sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, str_ip, (void *)(&(remote->sin_addr.s_addr)));
  	if( tmpres < 0)   {
    	perror("Can't set remote->sin_addr.s_addr");
    	fprintf(stderr, "%s\n", "PROBLEM");
  	} else if(tmpres == 0) {
    	/* fprintf(stderr, "%s is not a valid IP address\n", ip); */
    	fprintf(stderr, "%s\n", "PROBLEM");
  	}
  	remote->sin_port = htons(port);
 
  	if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
	    perror("Could not connect");
	    fprintf(stderr, "%s\n", "PROBLEM");
  	}
 
  	/*Send the query to the server*/
    sent = 0;
  	while(sent < strlen(frame1)) {
  		fprintf(stderr, "%s", frame1+sent);
    	tmpres = send(sock, frame1+sent, strlen(frame1)-sent, 0);
    	if(tmpres == -1){
    	  	perror("Can't send query");
	      	fprintf(stderr, "%s\n", "PROBLEM");
	    }
	    sent += tmpres;
  	}

  	sleep(1);

  	sent = 0;
  	while(sent < strlen(frame2)) {
  		fprintf(stderr, "%s", frame2+sent);
    	tmpres = send(sock, frame2+sent, strlen(frame2)-sent, 0);
    	if(tmpres == -1){
    	  	perror("Can't send query");
	      	fprintf(stderr, "%s\n", "PROBLEM");
	    }
	    sent += tmpres;
  	}
  	fprintf(stderr, "\n" );
  	/*now it is time to receive the page*/
  	memset(buf, 0, sizeof(buf));


  	while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
	    fprintf(stderr, "%s\n", buf);
  	}

  	free(remote);
  	close(sock);
	/* It would be good to make some kind of JSON validifier */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}
