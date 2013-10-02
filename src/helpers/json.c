
/* JSON Escaping function and encoding */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helpers/json.h"


/*This function assumes that the output is large enough to handle
 *whatever input is being escaped. For safety, you should allocate
 *about 4 times as much space if you want to be completely safe.
 *you could calculate how much additional space you need by noting 
 *how many of each escaped character you have, but hey, whose gonna 
 *do that? (write a function for it maybe?)
*/
int _escapeJSON(const char * input, int inputlen, char * output){
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

/* This is a 'safe' version of the gs_scopeToJSON function. It very deliberately
 * uses more than enough memory in its allocations and does it's best to detect
 * possible stack smashing. If the function determines that it might be smashing
 * the stack at all it prints an error message to stderr and returns -1 without
 * copying the created input to jsonOutput. Because of the use of snprint instead
 * of sprintf, this function should behave more stability than the other one and
 * should be used instead.
*/
int gs_scopeNToJSON(const struct gs_scope gss, char * jsonOutput, int jsonOutputAllocatedSize){
    #ifdef JSON_ID_LENGTH
        #undef JSON_ID_LENGTH
        #define JSON_ID_LENGTH (12+sizeof(long)+1)
    #else
        #define JSON_ID_LENGTH (12+sizeof(long)+1)
    #endif
    #ifdef JSON_DESCRIPTION_LENGTH
        #undef JSON_DESCRIPTION_LENGTH
        #define JSON_DESCRIPTION_LENGTH (23+GS_SCOPE_DESCRIPTION_SIZE+1)
    #else
        #define JSON_DESCRIPTION_LENGTH (23+GS_SCOPE_DESCRIPTION_SIZE+1)
    #endif
    char jsonId[JSON_ID_LENGTH];
    char jsonDescription[JSON_DESCRIPTION_LENGTH];
    char escaped[GS_SCOPE_DESCRIPTION_SIZE*4];
    bzero(escaped,GS_SCOPE_DESCRIPTION_SIZE*4);
    bzero(jsonId,JSON_ID_LENGTH);
    int jsonIdWritten;
    int jsonDescriptionWritten;
    
    jsonIdWritten = snprintf(jsonId,12+sizeof(long),"{\"id\" : %ld , ",gss.id);

    /* The most important part is escaping the text*/
    _escapeJSON(gss.description,strlen(gss.description), escaped );

    jsonDescriptionWritten = snprintf(jsonDescription,JSON_DESCRIPTION_LENGTH,"\"description\" : \"%s\" }",escaped);
    if(jsonIdWritten + jsonDescriptionWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_scopeNToJSON may have overwritten the stack.");
        return -1; /* Flag since normally we return the number of characters written */
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1,"%s%s",jsonId,jsonDescription);
    
}

/* Recommended at least 128 bytes for safety */
int gs_scopeToJSON(const struct gs_scope gss, char * jsonOutput){
	char * json;
    char escaped[33];
    bzero(escaped,33);
    
	json = "{\"id\" : %ld , \"description\" : \"%s\" }";
	/* The most important part is escaping the text*/
	_escapeJSON(gss.description,strlen(gss.description), escaped );

    return sprintf(jsonOutput, json, gss.id ,escaped);
}

/* Recommended at least 256 bytes for jsonOutput to be allocated */
int gs_commentToJSON(const struct gs_comment gsc, char * jsonOutput){
    char * json;
    char escaped[141];
    bzero(escaped,141);

    json = "{\"id\" : %ld, \"message\" : \"%s\", \"timestamp\" : \"%s\" }";
    _escapeJSON(gsc.content, strlen(gsc.content), escaped);

    return sprintf(jsonOutput, json, gsc.id, escaped, gsc.createdTime);
}

/* I'd recommend at least 110 bytes to be specified.  Probably 128 for safety*/
int gs_markerToJSON(const struct gs_marker gsm, char * jsonOutput){
    char * json;

    json = "{\"id\" : %ld, \"commentId\" : %ld, \"timestamp\" : \"%s\", \"latitude\" : %ld.%08lu, \"longitude\" : %ld.%08lu }";

    return sprintf( jsonOutput, 
                    json, 
                    gsm.id, 
                    gsm.commentId, 
                    gsm.createdTime, 
                    gsm.latitude.left, gsm.latitude.right, 
                    gsm.longitude.left, gsm.longitude.right);    
}

/* Recommend at least 128 for safety*/ 
int gs_heatmapToJSON(const struct gs_heatmap gsh, char * jsonOutput){
    char * json;

    json = "{\"latitude\" : %ld.%08lu, \"longitude\" : %ld.%08lu, \"secondsWorked\" : %ld }";

    return sprintf( jsonOutput,
                    json,
                    gsh.latitude.left, gsh.latitude.right,
                    gsh.longitude.left, gsh.longitude.right,
                    gsh.intensity
                    );
}


/* Recommed at least 67 + GS_REPORT_MAX_LENGTH*3 for safety for jsonOutput size*/
int gs_reportToJSON(const struct gs_report gsr, char * jsonOutput){
    char * jsonMsg;
    char * jsonPartial;
    char escaped[strlen(gsr.content)*3];
    char jsonPartialString[43+65+20];
    char jsonPartialMsg[strlen(gsr.content)*3+20];
    bzero(escaped,strlen(gsr.content)*3);
    bzero(jsonPartialMsg,strlen(gsr.content)*3+20);

    /* The hash is the auth hash not the origin hash*/
    jsonMsg = "{\"message\" : \"%s\", ";
    jsonPartial = "\"timestamp\" : \"%s\", \"hash\" : \"%s\" }";
    _escapeJSON(gsr.content, GS_REPORT_MAX_LENGTH*3, escaped);

    /* the message content can easily overrun the stack so try to be extra careful */
    snprintf(jsonPartialString,43+65+20,jsonPartial,gsr.createdTime, gsr.authorize);
    snprintf(jsonPartialMsg,strlen(gsr.content)*3+20,jsonMsg, escaped);
    

    return sprintf( jsonOutput,"%s%s", jsonPartialMsg, jsonPartialString);
}