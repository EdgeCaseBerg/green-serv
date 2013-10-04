
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
        fprintf(stderr, "%s\n", "gs_scopeNToJSON may have returned partial JSON output due to not allocating enough memory for output buffer");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
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
    char escaped[GS_COMMENT_MAX_LENGTH*3];
    bzero(escaped,GS_COMMENT_MAX_LENGTH*3);

    json = "{\"id\" : %ld, \"message\" : \"%s\", \"timestamp\" : \"%s\" }";
    _escapeJSON(gsc.content, strlen(gsc.content), escaped);

    return sprintf(jsonOutput, json, gsc.id, escaped, gsc.createdTime);
}

int gs_commentToNJSON(const struct gs_comment gsc, char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonId[15+sizeof(long)]; /*{\"id\" : %ld, */
    char jsonMessage[23+(GS_COMMENT_MAX_LENGTH*4)+1];/* \"message\" : \"%s\", */
    char jsonTimestamp[25+GS_COMMENT_CREATED_TIME_LENGTH+1];/* \"timestamp\" : \"%s\" }*/
    char escaped[GS_COMMENT_MAX_LENGTH*4+1];
    int jsonIdWritten;
    int jsonMessageWritten;
    int jsonTimestampWritten;
    bzero(jsonId,15+sizeof(long));
    bzero(jsonMessage,23+(GS_COMMENT_MAX_LENGTH*4));
    bzero(jsonTimestamp,25+GS_COMMENT_CREATED_TIME_LENGTH+1);
    
    jsonIdWritten = snprintf(jsonId,15+sizeof(long),"{\"id\" : %ld, ", gsc.id);
    jsonTimestampWritten = snprintf(jsonTimestamp,25+GS_COMMENT_CREATED_TIME_LENGTH+1," \"timestamp\" : \"%s\" }",gsc.createdTime);

    _escapeJSON(gsc.content, strlen(gsc.content), escaped);
    jsonMessageWritten = snprintf(jsonMessage,23+(GS_COMMENT_MAX_LENGTH*4)," \"message\" : \"%s\", ",escaped);
    
    if(jsonTimestampWritten + jsonIdWritten + jsonMessageWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_commentNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s%s%s", jsonId,jsonMessage,jsonTimestamp);

}


/* I'd recommend at least 110 bytes to be specified.  Probably 128 for safety*/
int gs_markerToJSON(const struct gs_marker gsm, char * jsonOutput){
    char * json;
    char latitude[16];
    char longitude[16];
    bzero(latitude,16);
    bzero(longitude,16);
    formatDecimal(gsm.latitude,latitude);
    formatDecimal(gsm.longitude,longitude);

    json = "{\"id\" : %ld, \"commentId\" : %ld, \"timestamp\" : \"%s\", \"latitude\" : %s, \"longitude\" : %s }";

    return sprintf( jsonOutput, 
                    json, 
                    gsm.id, 
                    gsm.commentId, 
                    gsm.createdTime, 
                    latitude,
                    longitude);    
}

int gs_markerNToJSON(const struct gs_marker gsm, char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonId[15+sizeof(long)]; /*{\"id\" : %ld, */
    char jsonCommId[21+sizeof(long)];/*\"commentId\" : %ld, */
    char jsonTimestamp[25+GS_MARKER_CREATED_TIME_LENGTH+1];/* \"timestamp\" : \"%s\", */
    char jsonLat[21+16];/* \"latitude\" : %s, */
    char jsonLon[21+16];/* \"longitude\" : %s }*/
    char latitude[16];
    char longitude[16];
    int jsonIdWritten;
    int jsonCommIdWritten;
    int jsonTimestampWritten;
    int jsonLatWritten;
    int jsonLonWritten;
    bzero(jsonId, 21+sizeof(long) );
    bzero(jsonCommId, 21+sizeof(long) );
    bzero(jsonTimestamp, 21+GS_MARKER_CREATED_TIME_LENGTH+1 );
    bzero(jsonLat, 21+16);
    bzero(jsonLon, 21+16 );   
    formatDecimal(gsm.latitude,latitude);
    formatDecimal(gsm.longitude,longitude);
    
    jsonIdWritten = snprintf(jsonId, 21+sizeof(long), "{\"id\" : %ld, ", gsm.id);
    jsonCommIdWritten = snprintf(jsonCommId, 21+sizeof(long), "\"commentId\" : %ld, ", gsm.commentId);
    jsonTimestampWritten = snprintf(jsonTimestamp, 21+GS_MARKER_CREATED_TIME_LENGTH, " \"timestamp\" : \"%s\", ", gsm.createdTime);
    jsonLatWritten = snprintf(jsonLat, 21+15, " \"latitude\" : %s, ", latitude);
    jsonLonWritten = snprintf(jsonLon, 21+15, " \"longitude\" : %s }", longitude);

    if(jsonIdWritten + jsonCommIdWritten + jsonTimestampWritten + jsonLonWritten + jsonLatWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_markerNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1,"%s%s%s%s%s",jsonId, jsonCommId,jsonTimestamp,jsonLat,jsonLon);
}



/* Recommend at least 128 for safety*/ 
int gs_heatmapToJSON(const struct gs_heatmap gsh, char * jsonOutput){
    char * json;
    char latitude[16];
    char longitude[16];
    bzero(latitude,16);
    bzero(longitude,16);
    formatDecimal(gsh.latitude,latitude);
    formatDecimal(gsh.longitude,longitude);

    json = "{\"latitude\" : %s, \"longitude\" : %s, \"secondsWorked\" : %ld }";

    return sprintf( jsonOutput,
                    json,
                    latitude,
                    longitude,
                    gsh.intensity
                    );
}

int gs_heatmapNToJSON(const struct gs_heatmap gsh, char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonLat[21];
    char jsonLon[22];
    char latitude[16];
    char longitude[16];
    char jsonSeconds[27+sizeof(long)];
    int jsonLatWritten;
    int jsonLonWritten;
    int jsonSecondsWritten;
    bzero(latitude,16);
    bzero(longitude,16);
    bzero(jsonLat,21);
    bzero(jsonLon,21);
    bzero(jsonSeconds,27+sizeof(long));
    formatDecimal(gsh.latitude,latitude);
    formatDecimal(gsh.longitude,longitude);

    jsonLatWritten = snprintf(jsonLat,21+16,"{\"latitude\" : %s", latitude);
    jsonLonWritten = snprintf(jsonLon,22+16," \"longitude\" : %s,", longitude);
    jsonSecondsWritten = snprintf(jsonSeconds,27+sizeof(long)," \"secondsWorked\" : %ld }",gsh.intensity);

    if(jsonLonWritten + jsonLatWritten + jsonSecondsWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_heatmapNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif   
    }
    return snprintf(jsonOutput, jsonOutputAllocatedSize-1,"%s%s%s",jsonLat, jsonLon, jsonSeconds);
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

/* 'Safe' gs_reportTOJSON
*/
int gs_reportNToJSON(const struct gs_report gsr, char * jsonOutput, int jsonOutputAllocatedSize){
    #ifndef JSON_CONTENT_LENGTH
        #define JSON_CONTENT_LENGTH (20+(GS_REPORT_MAX_LENGTH)*4+1)
    #else
        #undef JSON_CONTENT_LENGTH
        #define JSON_CONTENT_LENGTH (20+(GS_REPORT_MAX_LENGTH)*4+1)
    #endif
    char jsonMessage[JSON_CONTENT_LENGTH];
    #ifndef JSON_TIMESTAMP_LENGTH
        #define JSON_TIMESTAMP_LENGTH 20+(GS_REPORT_CREATED_TIME_LENGTH)+1
    #else
        #undef JSON_TIMESTAMP_LENGTH
        #define JSON_TIMESTAMP_LENGTH 20+(GS_REPORT_CREATED_TIME_LENGTH)+1
    #endif
    char jsonTimestamp[JSON_TIMESTAMP_LENGTH];
    #ifndef JSON_HASH_LENGTH
        #define JSON_HASH_LENGTH 16 + SHA_LENGTH +1
    #else
        #undef JSON_HASH_LENGTH
        #define JSON_HASH_LENGTH 16 + SHA_LENGTH +1
    #endif
    char jsonHash[JSON_HASH_LENGTH];
    char escaped[JSON_CONTENT_LENGTH];
    int jsonMessageWritten;
    int jsonTimestampWritten;
    int jsonHashWritten;
    bzero(jsonMessage,JSON_CONTENT_LENGTH);
    bzero(jsonTimestamp,JSON_TIMESTAMP_LENGTH);
    bzero(jsonHash,JSON_HASH_LENGTH);
    bzero(escaped,JSON_CONTENT_LENGTH);

    jsonTimestampWritten = snprintf(jsonTimestamp,JSON_TIMESTAMP_LENGTH,"\"timestamp\" : \"%s\",",gsr.createdTime);
    jsonHashWritten = snprintf(jsonHash, JSON_HASH_LENGTH," \"hash\" : \"%s\" }",gsr.authorize );

    _escapeJSON(gsr.content, strlen(gsr.content), escaped);

    jsonMessageWritten = snprintf(jsonMessage, JSON_CONTENT_LENGTH, "{\"message\" : \"%s\", ",escaped );
    
    if(jsonMessageWritten + jsonHashWritten + jsonTimestampWritten > jsonOutputAllocatedSize){
        fprintf(stderr, "%s\n", "gs_reportNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s%s%s", jsonMessage,jsonTimestamp,jsonHash);

}