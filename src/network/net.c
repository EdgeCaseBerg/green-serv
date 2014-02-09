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
#if(NETWORK_LOGGING != 2 && NETWORK_LOGGING != 1) 
    #undef NETWORK_LOGGING
    #define NETWORK_LOGGING 0
#endif
#define NETWORK_LOG_LEVEL_2_NUM(s,d) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s %d\n",(s), (d) );
#define NETWORK_LOG_LEVEL_2(s) if(NETWORK_LOGGING == 2) fprintf(stderr, "%s\n", (s) );
#define NETWORK_LOG_LEVEL_1(s) if(NETWORK_LOGGING >= 1) fprintf(stderr, "%s\n", (s) );

static inline void  swapCharPtr( char ** ptr1, char ** ptr2){
    char *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
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

/* Ha, this function name is great. DO NETWORK WORK -- doNetWork! 
 * Is funny because network iis what we talk over see?
*/
 #define STARTING_RESPONSE_SIZE 2000
void* doNetWork(struct threadData* td) {
    /*Response Variables*/
    struct http_request request;
    int bytesSent;
    int controller;
    int status;
    char * response;
    /* Request variables */
    int readAmount;
    int totalRead;
    int flags; 
    int j,k;
    int contentLength;
    int contentLengthRecieved;
    int contentLengthPosition;
    char * tempBuff;
    char * raw_message;
    char contentLengthBuff[100];

    response = malloc(sizeof(char)*STARTING_RESPONSE_SIZE);
    if(response == NULL){
        NETWORK_LOG_LEVEL_1("Fatal: Failed to allocate memory for response");
        response = "{}";
        status = 500;
        goto internal_err;
    }
    memset(response,0,sizeof(char)*STARTING_RESPONSE_SIZE);

    raw_message = malloc(1+sizeof(char)*BUFSIZ); /* Just enough space for the first read, to determine content length */
    if(raw_message == NULL){
        /* Internal Problem! */
        NETWORK_LOG_LEVEL_1("Fatal: Failed to allocate memory for first-read temporary buffer");
        response[0]='{'; response[1]='}'; response[2]='\0';
        status = 500;
        goto internal_err;
    }
    memset(raw_message, 0,1+sizeof(char)*BUFSIZ);
    tempBuff = NULL; /* temp buff will be used to realloc */
        

    /* Accept and read incoming request  */
    if(td->clientfd == -1)
        goto bad_client_id;

    readAmount=totalRead=contentLengthRecieved =contentLengthPosition= contentLength = 0;

    NETWORK_LOG_LEVEL_2_NUM("Accepted Client Request on File Descriptor ", td->clientfd);
    readAmount = 1; /* Sentinal */
    flags = fcntl(td->clientfd, F_GETFL, 0);
    fcntl(td->clientfd, F_SETFL, flags | O_NONBLOCK);
    contentLength = -1; /* Sentinal */
    while(readAmount != 0){
        readAmount = read(td->clientfd,raw_message+totalRead,BUFSIZ);    
        if(readAmount == -1){
            if(errno == EAGAIN && totalRead == 0)
                continue;
            else if(contentLengthRecieved != contentLength)
                continue;
            else
                readAmount = 0;
        }else{
            NETWORK_LOG_LEVEL_2_NUM("Reading Data From Socket", td->clientfd);
            /* Since we're here that means we have at least 
             * the beginning of the request itself but we're
             * waiting for more. So determine the content-length
             * and then figure out if we have all of it or not
            */
            contentLengthPosition = strnstr("Content-Length", raw_message, BUFSIZ);                    
            if(contentLengthPosition == -1){
                /* You didn't send us a content length, carry on! 
                 * so no data, so just url, so good bye.
                */
                contentLengthRecieved = contentLength = readAmount = 0;
            }else{
                if(totalRead == 0){                            
                    /* Convert the content length 
                    * reuse this connections buffer.
                    */
                    bzero(contentLengthBuff, sizeof contentLengthBuff); /* Only what we need */
                    contentLength = contentLengthPosition;
                    contentLength+=strlen("Content-Length: "); /* Skip the text */
                     
                    for(k=0; k < (int)sizeof(contentLengthBuff) && *(raw_message + contentLength) != '\0' && *(raw_message + contentLength) != '\r'; ++k, contentLength++)
                        contentLengthBuff[k] = *(raw_message + contentLength);
                    contentLengthBuff[k] = '\0';
                    contentLength = atoi(contentLengthBuff);                            
                    
                    /* Malloc for the content Length 
                     * j is the position of the data, all things prior 
                     * need to be conserved as well
                    */
                    if( contentLength > 0 ){
                        tempBuff = malloc(5+ strlen(raw_message) + contentLength + BUFSIZ);
                        if(tempBuff == NULL){
                            free(raw_message);
                            NETWORK_LOG_LEVEL_1("Could not reallocate memory during request data acquisition");
                            goto internal_err;
                        }else{
                            memset(tempBuff, 0, 5+ strlen(raw_message) + contentLength + BUFSIZ);
                            strcpy(tempBuff, raw_message);
                            swapCharPtr(&tempBuff, &raw_message);
                            free(tempBuff);
                        }
                    }
                }

                /* We've said a content length now, and we need 
                * determine how much we've actually recieved
                * no need to store it, just count it with j. 
                */
                j = strnstr("\r\n\r\n", raw_message, BUFSIZ);
                if(j != -1){
                    j+=4; /* skip newlines */
                    j+= contentLengthRecieved;
                    for(contentLengthRecieved = (contentLengthRecieved==0 ? 0 : contentLengthRecieved); (unsigned int)j < strlen(raw_message); ++j, ++contentLengthRecieved)
                        ;                            
                }else{   
                    /* Could not find content...*/
                    if(contentLength <= 0)
                        readAmount = 0; /* Get out */
                    else
                        contentLengthRecieved = 0; /* Haven't received it yet */
                }
            }
            /* Important to do this last as the totalRead is what
             * determines if we malloc for content or not
            */
            
            fprintf(stderr, "Recieving Data From Socket %d: %d/%d\n", td->clientfd ,contentLengthRecieved, contentLength);    
            
            totalRead += readAmount;                
            if(contentLength == contentLengthRecieved) 
                readAmount = 0;      
        }
    }
    if(totalRead > THREAD_DATA_MAX_SIZE)
        NETWORK_LOG_LEVEL_1("Warning: Total Read Greater than Buffer Length");

    /*Blank JSON response for no control*/
    *(response+1)='{'; *(response+1)='}'; *(response+2)='\0';
    status = 200;
    parseRequest(&request, raw_message);

    /* Pass the request off to a handler */
    controller = determineController(request.url);
    /* Determine method and call. */
    switch(controller){
        case HEARTBEAT_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Heartbeat Controller Processing Request.");
            status = heartbeat_controller(response, STARTING_RESPONSE_SIZE);
            break;
        case COMMENTS_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Comments Controller Processing Request.");
            status = comment_controller(&request, &response,  STARTING_RESPONSE_SIZE);
            break;
        case HEATMAP_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Heatmap Controller Processing Request.");
            status = heatmap_controller(&request, &response,  STARTING_RESPONSE_SIZE);
            break;
        case MARKER_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Marker Controller Processing Request.");
            status = marker_controller(&request, &response,  STARTING_RESPONSE_SIZE);
            break;
        case REPORT_CONTROLLER :
            NETWORK_LOG_LEVEL_2("Report Controller Processing Request.");
            status = report_controller(&request, &response,  STARTING_RESPONSE_SIZE);
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
    if(request.contentLength > 0){
        NETWORK_LOG_LEVEL_2("Incoming Data:");
        NETWORK_LOG_LEVEL_2_NUM("Content length of Data: ", request.contentLength);
        NETWORK_LOG_LEVEL_2(request.data);
    }
    if(request.contentLength > 0)
        free(request.data);

    internal_err:
    td->msg = malloc(strlen(response) + 256);
    if(td->msg == NULL){
        NETWORK_LOG_LEVEL_2_NUM("Fatal: Not enough memory for HTTP response", (int)strlen(response)+256);
        free(raw_message);
        free(response);
        close(td->clientfd);
        return NULL;
    }
    memset(td->msg,0,strlen(response)+256);
    createResponse(response,td->msg,status);
    td->msg[strlen(td->msg)] = '\0';

    bad_client_id: 
    if(td->clientfd != -1){
        bytesSent = send(td->clientfd,td->msg,strlen(td->msg),0);  
        NETWORK_LOG_LEVEL_1("Sending Response:");
        NETWORK_LOG_LEVEL_1(td->msg);
        NETWORK_LOG_LEVEL_2_NUM("Bytes sent to client: ", bytesSent);
        NETWORK_LOG_LEVEL_2(strerror(errno));
        close(td->clientfd);
    }else{
        NETWORK_LOG_LEVEL_2("File Descriptor invalid. If shutting down there is no problem.");
        NETWORK_LOG_LEVEL_2("If not shutting down, there was an issue sending data to the client.");
    }
    free(td->msg);
    free(raw_message);
    free(response);
    return NULL;
}


/*Create a HTTP response with the buff as content and sent out with the
 *response code of status. This function call is not safe.
*/
int createResponse(char * content, char * buff, int status){
    char timeBuffer[40];
    time_t now;
    struct tm tm;
    bzero(timeBuffer,40);
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
        for(i=contentLength; methodLoc < FIRSTLINEBUFFSIZE && requestStr[i] != '\0' && requestStr[i] != '\r'; ++i)
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
    /* Calling parties must use contentLength to tell if they will need to free the memory */
    requestToFill->contentLength = contentLength;
    return contentLength;

    
}

volatile sig_atomic_t stop;

void stop_server(int signum){
    /* Call me with ctrl Z if ctrl c doesnt work*/
    stop = 1;
    if(signum == SIGINT)
        stop =1;
}


int run_network(void*(*func)(void*)){
    struct sockaddr_in sockserv,sockclient;
    int clientfd;
    int socketfd;
    socklen_t clientsocklen;
    pthread_t children[NUMTHREADS];
    struct threadData data[NUMTHREADS];
    int i,j;
    stop = 0;
    #ifdef DETACHED_THREADS
    pthread_attr_t attr;
    #endif
    fd_set rfds;
    struct timeval tv;
    int retval;
    int sec;
    int usec;

    clientfd = socketfd  = 0; 
    
    bzero(&sockserv,sizeof(sockserv));
    sec = 0;
    usec = 10;



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

    FD_ZERO(&rfds);
    FD_SET(socketfd, &rfds);
    tv.tv_sec = sec;
    tv.tv_usec = usec;


    i=j=0;
    if(errno != 13){
        while(stop == 0){
            for(i=0; i < NUMTHREADS && stop == 0; i++){
                retval = select((socketfd+1)/*see "man select_tut"*/, &rfds, NULL, NULL, &tv);
                /* Reset select */
                tv.tv_sec = sec;
                tv.tv_usec = usec;
                if(retval == -1 || ! FD_ISSET(socketfd, &rfds)){
                    /* From select_tut: 
                     *
                     *   After select() has returned, readfds will  be  cleared
                     *   of all file descriptors except for those that are immediately 
                     *   available for reading.
                     *
                     *  Because we always want to monitor the setwe add the socket back in.
                     */
                    FD_SET(socketfd, &rfds);
                    i--;
                    continue;
                }
                FD_SET(socketfd, &rfds);

                /* We have yet to recieve anything or check for the header */
                clientfd = accept(socketfd,(struct sockaddr*)&sockclient,&clientsocklen);
                if(clientfd != -1){
                    data[i].clientfd = clientfd;
                    #ifndef DETACHED_THREADS
                        pthread_create(&children[i],NULL,func,&data[i]);
                    #else
                        NETWORK_LOG_LEVEL_2("Spawning detached thread");
                        pthread_create(&children[i],&attr,func,&data[i]);
                    #endif
                }else{
                    NETWORK_LOG_LEVEL_1("Connection shutdown.");
                    NETWORK_LOG_LEVEL_2("Invalid file descriptor from client connection.");
                    NETWORK_LOG_LEVEL_2("If shutting down server ignore previous warning.");
                    i--; /* Move thread index back one */
                }                
            }           
            /*Gobble Up the resources (if not detaching threads)
             *If you do want to detach threads change the define. 
             *in net.h
            */
            #ifndef DETACHED_THREADS
            NETWORK_LOG_LEVEL_1("Pausing to Join threads. One moment...");
            for(j=0; j < NUMTHREADS && j < i; ++j){
                pthread_join(children[j],NULL);
            }
            #endif
        }
    }
    #ifdef DETACHED_THREADS
    pthread_attr_destroy(&attr);
    #endif
    close(socketfd);
    BOOT_LOG_STR("Exiting Server...", "");
    wait(NULL);
    return 0;
}

#undef NETWORK_LOG_LEVEL_2_NUM
#undef NETWORK_LOG_LEVEL_2
#undef NETWORK_LOG_LEVEL_1
#undef BOOT_LOG_STR