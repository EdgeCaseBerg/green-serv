
/* JSON Escaping function and encoding */
#include "scope.h"
#include "comment.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*This function assumes that the output is large enough to handle
 *whatever input is being escaped. For safety, you should allocate
 *about 4 times as much space if you want to be completely safe.
 *you could calculate how much additional space you need by noting 
 *how many of each escaped character you have, but hey, whose gonna 
 *do that? (write a function for it maybe?)
*/
int _escapeJSON(char * input, int inputlen, char * output){
    int i,j;
    for(i=j=0; input[i] != '\0' && i < inputlen; ++i,++j ){
        switch(input[i]){
            case '\\':
                output[j] = '\\';
                output[++j] = '\\';
                break;
            case '"':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = '\\';
                output[++j] = '"';
                break;
            case '/':
                output[j] = '\\';
                output[++j] = '\\'; 
                output[++j] = '/';
                break;
            case '\b':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = 'b';
                break;
            case '\f':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = 'f';
                break;
            case '\n':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = 'n';
                break;
            case '\r':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = 'r';
                break;
            case '\t':
                output[j] = '\\';
                output[++j] = '\\';
                output[++j] = 't';
                break;
            default:
                output[j] = input[i];
        }
    }
    output[j] = '\0';
    return (j+1);
}

int gsScopeToJSON(struct gs_scope gss, char * jsonOutput){
	char * json;
    char escaped[33];
    bzero(escaped,33);
    
	json = "{\"id\" : %ld , \"description\" : \"%s\" }";
	/* The most important part is escaping the text*/
	_escapeJSON(gss.description,strlen(gss.description), escaped );

    return sprintf(jsonOutput, json, gss.id ,escaped);
}


int gsCommentToJSON(struct gs_comment gsc, char * jsonOutput){
    char * json;
    char escaped[141];
    bzero(escaped,141);

    json = "{\"id\" : %ld, \"message\" : \"%s\", \"timestamp\" : \"%s\" }";
    _escapeJSON(gsc.content, strlen(gsc.content), escaped);

    return sprintf(jsonOutput, json, gsc.id, escaped, gsc.createdTime);
}

/* I'd recommend at least 110 bytes to be specified.  Probably 128 for safety*/
int gsMarkerToJSON(struct gs_marker gsm, char * jsonOutput){
    char * json;

    json = "{\"id\" : %ld, \"commentId\" : %ld, \"timestamp\" : \"%s\", \"latitude\" : %ld.%lu, \"longitude\" : %ld.%lu }";

    return sprintf( jsonOutput, 
                    json, 
                    gsm.id, 
                    gsm.commentId, 
                    gsm.createdTime, 
                    gsm.latitude.left, gsm.latitude.right, 
                    gsm.longitude.left, gsm.longitude.right);    
}