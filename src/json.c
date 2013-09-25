
/* JSON Escaping function and encoding */
#include "scope.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
