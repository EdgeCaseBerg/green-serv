#include "network/net.h"

#ifndef BOOT_LOGGING
    #define BOOT_LOGGING 0
#endif
#if(BOOT_LOGGING != 1)
    #ifdef BOOT_LOGGING
        #undef BOOT_LOGGING
    #endif
    #define BOOT_LOGGING 0
#endif
/*pre must be a string declared like "sting" not a variable*/
#define BOOT_LOG_STR(pre,s) if(BOOT_LOGGING == 1) fprintf(stderr, pre "%s\n", (s));

#ifndef NETWORK_LOGGING
   #define NETWORK_LOGGING 0
#endif
#if(NETWORK_LOGGING != 2 || NETWORK_LOGGING != 1) 
    #undef NETWORK_LOGGING
    #define NETWORK_LOGGING 0
#endif
#define NETWORK_LOG_LEVEL_2_NUM(s,d) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s %d\n",(s), (d) );
#define NETWORK_LOG_LEVEL_2(s) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s\n", (s) );
#define NETWORK_LOG_LEVEL_1(s) if(NETWORK_LOGGING >= 1) fprintf(stderr, "%s\n", (s) );

/* Ha, this function name is great. DO NETWORK WORK -- doNetWork! 
 * Is funny because network is what we talk over see?
*/
void* doNetWork(struct threadData* td) {
    /* Copy local data 
    struct ThreadData* data=(struct ThreadData*) td;*/
    struct http_request request;
    int bytesSent;
    int controller;
    int status;
    char response[10000];
    bzero(response, sizeof response);

    /*Blank JSON response for no control*/
    response[0]='{'; response[1]='}'; response[2]='\0';
    status = 200;
    parseRequest(&request, td->msg);

    /* Pass the request off to a handler */
    controller = determineController(request.url);
    /* Determine method and call. */
    switch(controller){
        case HEARTBEAT_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Heartbeat Controller Processing Request.");
            status = heartbeat_controller(response,sizeof response);
            break;
        case COMMENTS_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Comments Controller Processing Request.");
            status = comment_controller(&request, response, sizeof response);
            break;
        case HEATMAP_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Heatmap Controller Processing Request.");
            status = heatmap_controller(&request, response, sizeof response);
            break;
        case MARKER_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Marker Controller Processing Request.");
            status = marker_controller(&request, response, sizeof response);
            break;
        case REPORT_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Report Controller Processing Request.");
            status = report_controller(&request, response, sizeof response);
            break;
        default:
            NETWORK_LOG_LEVEL_2("Unknown URL. Refusing to process request.");
            /* We have no clue what the client is talking about with their url */
            status = 404;
            break;
    }
    

    /* Log and clean up. */
    NETWORK_LOG_LEVEL_1("Incoming Request:")
    NETWORK_LOG_LEVEL_1(request.url);
    if(request.contentLength > 0)
        NETWORK_LOG_LEVEL_2("Incoming Data:");
        NETWORK_LOG_LEVEL_2(request.data);
    if(request.contentLength > 0)
        free(request.data);


    createResponse(response,td->msg,status);
    td->msg[strlen(td->msg)] = '\0';


    bytesSent = send(td->clientfd,td->msg,strlen(td->msg),0);  
    NETWORK_LOG_LEVEL_1("Sending Response:");
    NETWORK_LOG_LEVEL_1(td->msg);
    NETWORK_LOG_LEVEL_2_NUM("Bytes sent to client: ", bytesSent);
    NETWORK_LOG_LEVEL_2(strerror(errno));

    close(td->clientfd);

    return NULL;
}


/*Create a HTTP response with the buff as content and sent out with the
 *response code of status. This function call is not safe.
*/
int createResponse(char * content, char * buff, int status){
    char timeBuffer[1000];
    time_t now;
    struct tm tm;
    bzero(timeBuffer,1000);
    now = time(NULL);

    tm  = *gmtime(&now);
    strftime(timeBuffer, sizeof timeBuffer, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return sprintf(buff,  
                    "HTTP/1.1 %d OK\r\n"
                   "Content-Type: application/json\r\n"
                   "%sr\n"
                   "Connection: close\r\n"
                   "Content-Length: %zu\r\n\r\n%s\r\n",
                   status,
                   timeBuffer,
                   strlen(content),
                   content);
}

/*Create Socket for communications
 *returns file descriptor or -1 on err
*/
int createSocket(){
    /* perhaps SOCK_NONBLOCK one day... */
    int s;
    int optval;
    optval = 1;
    s = socket(AF_INET,SOCK_STREAM,0);   
    if(s != -1)
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    return s;
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
    sockserv->sin_port = htons(port);
    return bind(fd,(struct sockaddr *)sockserv,sizeof(*sockserv));
}

static int strnstr(char * needle, char * haystack, int haystackLen){
    int i;
    int j;
    
    int needleLen = strlen(needle);
    for(i=0; i < haystackLen && haystack[i] != '\0'; ++i){
        if( needle[0] == haystack[i] ){
            j=i;
            while(needle[j-i] == haystack[j] && j-i < needleLen)
                j++;
            if(j-i == needleLen){
                /* We made it through the haystack and
                 * and looked at all the characters without
                 * terminating early. 
                */
                 return i;
            }
        }
    }
    return -1;
}

int parseRequest(struct http_request * requestToFill, char * requestStr){
    char buff[FIRSTLINEBUFFSIZE]; /* This is the most we'll read */
    int i;
    int methodLoc;
    int urlEnd;
    int contentLength;
    bzero(buff,FIRSTLINEBUFFSIZE);
    bzero(requestToFill->url,MAX_URL_LENGTH);
    requestToFill->url[0] = '/'; /* Default assumption */

    /* Attempt to determine the request method */
    for(i=0; i <  FIRSTLINEBUFFSIZE && requestStr[i] != '\0'; ++i){
        if(requestStr[i] == '\r')
            if(requestStr[i+1] == '\n')
                break;
        /* Copy copy copy... */
        buff[i] = requestStr[i];
    }
    methodLoc = strnstr("GET", buff, FIRSTLINEBUFFSIZE  );
    if(methodLoc == -1){
        methodLoc = strnstr("POST", buff, FIRSTLINEBUFFSIZE );
        if(methodLoc == -1){
            methodLoc = strnstr("PUT", buff, FIRSTLINEBUFFSIZE  );
            if(methodLoc == -1){
                methodLoc = strnstr("DELETE", buff, FIRSTLINEBUFFSIZE   );
                if(methodLoc == -1){
                    /* Fuck it. What'd you try to give me? */
                    NETWORK_LOG_LEVEL_1("Could not determine HTTP method. Method Not Recognized:");
                    NETWORK_LOG_LEVEL_2(buff);
                    requestToFill->method = UNKNOWN_METHOD;
                }else{
                    requestToFill->method = DELETE;
                    methodLoc+= 7; /* Advance past the method and space */
                }
            }else{
                requestToFill->method = PUT;
                methodLoc+= 4;
            }
        }else{
            requestToFill->method = POST;
            methodLoc+=5;
        }
    }else{
        requestToFill->method = GET;
        methodLoc+=4;
    }
    urlEnd = strnstr("HTTP", buff, FIRSTLINEBUFFSIZE );
    /* Find the url (Which will be between the method to the http version)*/
    i=methodLoc;
    for(methodLoc=0; methodLoc < MAX_URL_LENGTH && i < FIRSTLINEBUFFSIZE && requestStr[i] != '\0' && i < urlEnd; ++i){
        if(requestStr[i] != ' '){
            requestToFill->url[methodLoc] = requestStr[i];
            methodLoc++; /* Just re-using a variable instead of using a new one */
        }
    }
    /* Find out if there's any content: */
    contentLength = strnstr("Content-Length",requestStr,strlen(requestStr));
    if(contentLength != -1){
        /*Determine actual content length */
        contentLength+=strlen("Content-Length: "); /* Skip the text */
        methodLoc=0; /* ReUsing again */
        for(i=contentLength; i < FIRSTLINEBUFFSIZE && requestStr[i] != '\0' && requestStr[i] != '\r'; ++i)
            buff[methodLoc++] = requestStr[i];
        buff[i] = '\0';
        /* Attempt to parse: */
        contentLength = atoi(buff);

        if(contentLength > 0){
            /* If there's data than place it into the structure */
            if(requestToFill->method == POST || requestToFill->method == PUT){
                requestToFill->data = malloc(contentLength*sizeof(char)+1);
                if(requestToFill->data != NULL){
                    /* Find the content: */
                    i = strnstr("\r\n\r\n", requestStr, strlen(requestStr));
                    if(i != -1){
                        i+=4; /* skip newlines */
                        for(methodLoc = 0; i < (int)strlen(requestStr) && requestStr[i] != '\0'; ++i, ++methodLoc)
                            requestToFill->data[methodLoc] = requestStr[i];
                        requestToFill->data[methodLoc] = '\0';
                    }else{
                        /* Could not find content...*/
                        free(requestToFill->data);
                    }
                }else{
                    NETWORK_LOG_LEVEL_1("Could not allocate enough memory for content in request");
                }
            }else{
                /* Don't preallocate memory for the struct or this will cause
                 * memory not being able to be reached
                */
                requestToFill->data = NULL;
            }
        }
    }
    /* Calling parties must use contentLenght to tell if they will need to free the memory */
    requestToFill->contentLength = contentLength;
    return contentLength;

    
}

/* Just a test function... */
int test_network(char * buffer, int bufferLength, void*(*func)(void*)){
    struct sockaddr_in sockserv,sockclient;
    int socketfd,clientfd;
    socklen_t clientsocklen;
    char buff[BUFSIZ];
    pthread_t children[NUMTHREADS];
    struct threadData data[NUMTHREADS];
    int i,j;
    #ifdef DETACHED_THREADS
    pthread_attr_t attr;
    #endif
    
    clientfd = socketfd = 0; 
    bzero(buff,BUFSIZ);
    bzero(&sockserv,sizeof(sockserv));

    socketfd = createSocket();
    printf("Socket Creation: %s\n",strerror(errno));

    setupSockAndBind(socketfd, &sockserv, PORT); 
    printf("Socket Bind: %s\n",strerror(errno));

    listen(socketfd,NUMTHREADS);
    printf("Socket Listen: %s\n%d\n",strerror(errno),errno);

    clientsocklen = sizeof socketfd;

    #ifdef DETACHED_THREADS
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    #endif

    i=j=0;
    if(errno != 13){
        for(j=0; j < 10; ++j){
            for(i=0; i < NUMTHREADS; i++){
                clientfd = accept(socketfd,(struct sockaddr*)&sockclient,&clientsocklen);
                printf("request accepted\n");

                buff[read(clientfd,buff,BUFSIZ)] = '\0';
                printf("Request recieved as \n%s\n",buff);  

                strncpy(buffer, buff ,bufferLength);

                /* A thread pool would be intelligent here */
                sprintf(data[i].msg, "%s", buffer);
                data[i].clientfd = clientfd;
                #ifndef DETACHED_THREADS
                    pthread_create(&children[i],NULL,func,&data[i]);
                #else
                    pthread_create(&children[i],&attr,func,&data[i]);
                #endif
                bzero(buff,BUFSIZ);
                bzero(buffer,bufferLength);
                
            }           
            /*Gobble Up the resources (if not detaching threads)
             *If you do want to detach threads change the define. 
             *in net.h
            */
            #ifndef DETACHED_THREADS
            for(i=0; i < NUMTHREADS; ++i)
                pthread_join(children[i],NULL);
            #endif
        }
    }
    #ifdef DETACHED_THREADS
    pthread_attr_destroy(&attr);
    #endif
    if(shutdown(socketfd,2) <0)
        fprintf(stderr, "%s\n", "Problem shutting down socket descriptor");
    if(close(socketfd) < 0)
        fprintf(stderr, "%s\n", "Problem closing socket descriptor");
    /* Sleep a moment to hope that any running threads will finish */
    fprintf(stdout, "%s\n", "Exiting Server...");
    sleep(2);

    return 0;
}


#include <unistd.h>
#include <stdio.h>
#include <signal.h>

volatile sig_atomic_t stop;
void stop_server(int signum){
    /* Call me with ctrl Z if ctrl c doesnt work*/
    stop = 1;
    if(signum == SIGINT)
        stop =1;
    
}

int run_network(char * buffer, int bufferLength, void*(*func)(void*)){
    struct sockaddr_in sockserv,sockclient;
    int socketfd,clientfd;
    socklen_t clientsocklen;
    char buff[BUFSIZ];
    pthread_t children[NUMTHREADS];
    struct threadData data[NUMTHREADS];
    int i,j;
    stop = 0;
    #ifdef DETACHED_THREADS
    pthread_attr_t attr;
    #endif
    
    clientfd = socketfd = 0; 
    bzero(buff,BUFSIZ);
    bzero(&sockserv,sizeof(sockserv));

    socketfd = createSocket();
    BOOT_LOG_STR("Socket Creation: ", strerror(errno));
    setupSockAndBind(socketfd, &sockserv, PORT); 
    BOOT_LOG_STR("Socket Bind: ", strerror(errno));
    listen(socketfd,NUMTHREADS);
    BOOT_LOG_STR("Socket Listen: ", strerror(errno));

    clientsocklen = sizeof socketfd;

    #ifdef DETACHED_THREADS
    BOOT_LOG_STR("Detached threads is defined", "");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    #endif

    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);
    signal(SIGQUIT, stop_server);
    signal(SIGHUP, stop_server);    

    i=j=0;
    if(errno != 13){
        while(stop == 0){
            for(i=0; i < NUMTHREADS && stop == 0; i++){
                clientfd = accept(socketfd,(struct sockaddr*)&sockclient,&clientsocklen);
                NETWORK_LOG_LEVEL_2_NUM("Accepted Client Request on File Descriptor ", clientfd);
                buff[read(clientfd,buff,BUFSIZ)] = '\0';
                strncpy(buffer, buff ,bufferLength);

                /* A thread pool would be intelligent here */
                sprintf(data[i].msg, "%s", buffer);
                data[i].clientfd = clientfd;
                #ifndef DETACHED_THREADS
                    pthread_create(&children[i],NULL,func,&data[i]);
                #else
                    NETWORK_LOG_LEVEL_2("Spawning detached thread");
                    pthread_create(&children[i],&attr,func,&data[i]);
                #endif
                bzero(buff,BUFSIZ);
                bzero(buffer,bufferLength);
                
            }           
            /*Gobble Up the resources (if not detaching threads)
             *If you do want to detach threads change the define. 
             *in net.h
            */
            #ifndef DETACHED_THREADS
            for(j=0; j < NUMTHREADS && j < i; ++j)
                NETWORK_LOG_LEVEL_1("Pausing to Join threads. One moment...");
                pthread_join(children[j],NULL);
            #endif
        }
    }
    #ifdef DETACHED_THREADS
    pthread_attr_destroy(&attr);
    #endif
    if(shutdown(socketfd,2) <0)
        fprintf(stderr, "%s\n", "Problem shutting down socket descriptor");
    if(close(socketfd) < 0)
        fprintf(stderr, "%s\n", "Problem closing socket descriptor");
    /* Sleep a moment to hope that any running threads will finish */
    BOOT_LOG_STR("Exiting Server...", "");
    sleep(2);

    return 0;
}

#undef NETWORK_LOG_LEVEL_2_NUM
#undef NETWORK_LOG_LEVEL_2
#undef NETWORK_LOG_LEVEL_1
#undef BOOT_LOG_STR