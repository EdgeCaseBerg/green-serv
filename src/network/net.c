#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>


int createResponse(char * content, char * buff){
    char timeBuffer[1000];
    time_t now;
    struct tm tm;
    bzero(timeBuffer,1000);
    now = time(NULL);

    tm  = *gmtime(&now);
    strftime(timeBuffer, sizeof timeBuffer, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return sprintf(buff,  
                    "HTTP/1.1 200 OK\r\n"
                   "Content-Type: application/json\r\n"
                   "%sr\n"
                   "Connection: close\r\n"
                   "Content-Length: %zu\r\n\r\n%s\r\n",
                   timeBuffer,
                   strlen(content),
                   content);
}

/*Create Socket for communications
 *returns file descriptor or -1 on err
*/
int createSocket(){
    /* perhaps SOCK_NONBLOCK one day... */
    return socket(AF_INET,SOCK_STREAM,0);   
}

/* Sets up a socket for use as a server socket on the designated port
 * returns -1 on errors, 0 otherwise
 * fd is the file descriptor of the socket to setup
 * sockserv is a pointer to a sockaddr_in struct.
 * port is the port to bind the address to
*/
int setupSockAndBind(int fd, struct sockaddr_in * sockserv, int port ){
    sockserv->sin_family = AF_INET;
    sockserv->sin_addr.s_addr = INADDR_ANY;
    sockserv->sin_port = htons(80);
    return bind(fd,(struct sockaddr *)sockserv,sizeof(*sockserv));
}

/* Just a test function... */
int test(){
    struct sockaddr_in sockserv,sockclient;
    int socketfd,clientfd;
    socklen_t clientsocklen;
    int bytesSent = clientfd = socketfd = 0; 
    char buff[BUFSIZ];

    bzero(buff,BUFSIZ);
    bzero(&sockserv,sizeof(sockserv));

    socketfd = createSocket();
    printf("Socket Creation: %s\n",strerror(errno));

    setupSockAndBind(socketfd, &sockserv, 80); 
    printf("Socket Bind: %s\n",strerror(errno));

    listen(socketfd,10);
    printf("Socket Listen: %s\n%d\n",strerror(errno),errno);

    clientsocklen = sizeof socketfd;
    if(errno != 13){
        clientfd = accept(socketfd,(struct sockaddr*)&sockclient,&clientsocklen);
        printf("request accepted\n");

        buff[read(clientfd,buff,BUFSIZ)] = '\0';
        printf("Request recieved as \n%s\n",buff);  

        createResponse("{}",buff);
        buff[strlen(buff)] = '\0';

        bytesSent = send(clientfd,buff,strlen(buff),0);  
        printf("Sent %d bytes to the client : %s\n",bytesSent,strerror(errno));

        close(clientfd);
            
    }
    close(socketfd);

    return 0;
}