    
/* JSON Escaping function and encoding */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helpers/json.h"

/*  validate JSON for opening and closing objects/arrays
*/
int validateJSON(const char * input, int input_length){
    int objects;
    int arrays;
    int i;
    objects=arrays=0;

    for( i=0; i < input_length && input[i] != '\0'; ++i ){
        if(input[i] == '{')
            objects++;
        if(input[i] == '}')
            objects--;
        if(input[i] == '[')
            arrays++;
        if(input[i] == ']')
            arrays--;
    }
    return objects == 0 && arrays == 0;
}   

/* Places all key pairs into strmap, all lowercase keys */
void parseJSON(const char * input, int input_length, StrMap * sm){
    int strFlag, i, j;
    char keyBuffer[256];
    char valBuffer[256];
    bzero(keyBuffer, sizeof keyBuffer);
    bzero(valBuffer, sizeof valBuffer);


    for(strFlag=i=0; i < input_length && input[i] != '\0'; ++i){
        /*We're at the start of a string*/
        if(input[i] == '"'){
            /*Go until we hit the closing qoute*/
            i++;
            for(j=0; i < input_length && input[i] != '\0' && input[i] != '"' && (unsigned int)j < sizeof keyBuffer; ++j,++i){
                keyBuffer[j] = (int)input[i] > 64 && input[i] < 91 ? input[i] + 32 : input[i]; /* Lowercase Keys, only ascii */
            }
            keyBuffer[j] = '\0';
            /*find the beginning of the value
             *which is either a " or a number. So skip spaces and commas
            */
            for(i++; i<  (int)(sizeof valBuffer)-1 && i < input_length && input[i] != '\0' && (input[i] == ',' || input[i] == ' ' || input[i] == ':'); ++i)
                ;
            /*Skip any opening qoute */
            if( i < (int)(sizeof valBuffer)-1 && input[i] != '\0' && input[i] == '"'){
                i++;
                strFlag = 1;
            }
            for(j=0; i < input_length && input[i] != '\0' && i < (int)(sizeof valBuffer)-1; ++j,++i){
                if(strFlag == 0){
                    if(input[i] == ' ' || input[i] == '\n' || input[i] == '}' || input[i] == ',')
                        break; /*break out if num data*/
                }else{
                    if( ( input[i] == '"' && input[i-1] != '\\') || input[i] == '}' )
                        break;
                }
                valBuffer[j] = input[i];
            }   
            valBuffer[j] = '\0';
            /* Skip any closing paren. */
            if(i < (int)(sizeof valBuffer) && input[i] == '"')
                i++;
            if(strlen(keyBuffer) > 0){
                if(sm_put(sm, keyBuffer, valBuffer) == 0)
                    fprintf(stderr, "Failed to copy parameters into hash table while parsing JSON Data: Key: %s, Val: %s\n", keyBuffer, valBuffer);
            }
        }
        strFlag = 0;
    }

}

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
    bzero(escaped,sizeof escaped);
    
	json = "{\"id\" : %ld , \"description\" : \"%s\" }";
	/* The most important part is escaping the text*/
	_escapeJSON(gss.description,strlen(gss.description), escaped );

    return sprintf(jsonOutput, json, gss.id ,escaped);
}

/* Recommended at least 256 bytes for jsonOutput to be allocated */
int gs_commentToJSON(const struct gs_comment gsc, char * jsonOutput){
    char * json;
    char escaped[GS_COMMENT_MAX_LENGTH*3];
    bzero(escaped,sizeof escaped);

    json = "{\"id\" : %ld, \"type\":\"%s\",\"pin\" : %ld,\"message\" : \"%s\", \"timestamp\" : \"%s\" }";
    _escapeJSON(gsc.content, strlen(gsc.content), escaped);

    return sprintf(jsonOutput, json, gsc.id, gsc.cType,gsc.pinId ,escaped, gsc.createdTime);
}

int gs_commentToNJSON(const struct gs_comment gsc, char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonId[15+sizeof(long)]; /*{\"id\" : %ld, */
    char jsonType[16+GS_COMMENT_TYPE_LENGTH];
    char jsonPinId[10+sizeof(long)]; /*"pinId" : %ld, */
    char jsonMessage[23+(GS_COMMENT_MAX_LENGTH*4)+1];/* \"message\" : \"%s\", */
    char jsonTimestamp[25+GS_COMMENT_CREATED_TIME_LENGTH+1];/* \"timestamp\" : \"%s\" }*/
    char escaped[GS_COMMENT_MAX_LENGTH*4+1];
    int jsonIdWritten;
    int jsonTypeWritten;
    int jsonPidIdWritten;
    int jsonMessageWritten;
    int jsonTimestampWritten;
    bzero(jsonId, sizeof jsonId);
    bzero(jsonType, sizeof jsonType);
    bzero(jsonPinId, sizeof jsonPinId);
    bzero(jsonMessage,sizeof jsonMessage);
    bzero(jsonTimestamp,sizeof jsonTimestamp);
    
    jsonIdWritten = snprintf(jsonId,sizeof jsonId,"{\"id\" : %ld, ", gsc.id);
    jsonTypeWritten = snprintf(jsonType, sizeof jsonType, "\"type\":\"%s\",", gsc.cType);
    jsonPidIdWritten = snprintf(jsonPinId, sizeof jsonPinId, "\"pin\" : %ld,", gsc.pinId);
    jsonTimestampWritten = snprintf(jsonTimestamp,25+GS_COMMENT_CREATED_TIME_LENGTH+1," \"timestamp\" : \"%s\" }",gsc.createdTime);

    _escapeJSON(gsc.content, strlen(gsc.content), escaped);
    jsonMessageWritten = snprintf(jsonMessage,23+(GS_COMMENT_MAX_LENGTH*4)," \"message\" : \"%s\", ",escaped);
    
    if(jsonTimestampWritten + jsonPidIdWritten + jsonIdWritten + jsonMessageWritten +jsonTypeWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_commentNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s%s%s%s%s", jsonId,jsonType,jsonPinId,jsonMessage,jsonTimestamp);

}


/* I'd recommend at least 110 bytes to be specified.  Probably 128 for safety*/
int gs_markerToJSON(const struct gs_marker gsm, char * jsonOutput){
    char * json;
    char latitude[DecimalWidth];
    char longitude[DecimalWidth];
    bzero(latitude,DecimalWidth);
    bzero(longitude,DecimalWidth);
    formatDecimal(gsm.latitude,latitude);
    formatDecimal(gsm.longitude,longitude);

    json = "{\"id\" : %ld, \"commentId\" : %ld, \"timestamp\" : \"%s\", \"latitude\" : %s, \"longitude\" : %s, \"addressed\" : %s }";

    return sprintf( jsonOutput, 
                    json, 
                    gsm.id, 
                    gsm.commentId, 
                    gsm.createdTime, 
                    latitude,
                    longitude,
                    gsm.addressed == ADDRESSED_TRUE ? "true" : "false");    
}

int gs_markerNToJSON(const struct gs_marker gsm, char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonId[15+sizeof(long)]; /*{\"id\" : %ld, */
    char jsonCommId[21+sizeof(long)];/*\"commentId\" : %ld, */
    char jsonTimestamp[25+GS_MARKER_CREATED_TIME_LENGTH+1];/* \"timestamp\" : \"%s\", */
    char jsonLat[21+DecimalWidth];/* \"latitude\" : %s, */
    char jsonLon[21+DecimalWidth];/* \"longitude\" : %s }*/
    char latitude[DecimalWidth];
    char longitude[DecimalWidth];
    char addressed[25]; 
    int jsonIdWritten;
    int jsonCommIdWritten;
    int jsonTimestampWritten;
    int jsonLatWritten;
    int jsonLonWritten;
    int jsonAddressedWritten;
    bzero(jsonId, sizeof jsonId );
    bzero(jsonCommId, sizeof jsonCommId );
    bzero(jsonTimestamp, sizeof jsonTimestamp );
    bzero(jsonLat, sizeof jsonLat);
    bzero(jsonLon, sizeof jsonLon );   
    bzero(addressed, sizeof addressed);
    bzero(latitude, sizeof latitude);
    bzero(longitude, sizeof longitude);
    formatDecimal(gsm.latitude,latitude);
    formatDecimal(gsm.longitude,longitude);

    printf(DecimalFormat "---%s\n", gsm.latitude ,latitude);
    
    jsonIdWritten = snprintf(jsonId, sizeof jsonId, "{\"id\" : %ld, ", gsm.id);
    jsonCommIdWritten = snprintf(jsonCommId, sizeof jsonCommId, "\"commentId\" : %ld, ", gsm.commentId);
    jsonTimestampWritten = snprintf(jsonTimestamp, sizeof jsonTimestamp, " \"timestamp\" : \"%s\", ", gsm.createdTime);
    jsonLatWritten = snprintf(jsonLat, sizeof jsonLat, " \"latitude\" : %s, ", latitude);
    jsonLonWritten = snprintf(jsonLon, sizeof jsonLon, " \"longitude\" : %s,", longitude);
    jsonAddressedWritten = snprintf(addressed, sizeof addressed, "\"addressed\" : %s}", gsm.addressed == ADDRESSED_TRUE ? "true" : "false");

    if(jsonIdWritten + jsonCommIdWritten + jsonTimestampWritten + jsonLonWritten + jsonLatWritten + jsonAddressedWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_markerNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1,"%s%s%s%s%s%s",jsonId, jsonCommId,jsonTimestamp,jsonLat,jsonLon, addressed);
}



/* Recommend at least 128 for safety*/ 
int gs_heatmapToJSON(const struct gs_heatmap gsh, char * jsonOutput){
    char * json;
    char latitude[DecimalWidth];
    char longitude[DecimalWidth];
    bzero(latitude,DecimalWidth);
    bzero(longitude,DecimalWidth);
    formatDecimal(gsh.latitude,latitude);
    formatDecimal(gsh.longitude,longitude);

    json = "{\"latDegrees\" : %s, \"lonDegrees\" : %s, \"secondsWorked\" : %ld }";

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
    char latitude[DecimalWidth];
    char longitude[DecimalWidth];
    char jsonSeconds[27+sizeof(long)];
    int jsonLatWritten;
    int jsonLonWritten;
    int jsonSecondsWritten;
    bzero(latitude,DecimalWidth);
    bzero(longitude,DecimalWidth);
    bzero(jsonLat,21);
    bzero(jsonLon,21);
    bzero(jsonSeconds,27+sizeof(long));
    formatDecimal(gsh.latitude,latitude);
    formatDecimal(gsh.longitude,longitude);

    jsonLatWritten = snprintf(jsonLat,21+DecimalWidth,"{\"latDegrees\" : %s,", latitude);
    jsonLonWritten = snprintf(jsonLon,22+DecimalWidth," \"lonDegrees\" : %s,", longitude);
    jsonSecondsWritten = snprintf(jsonSeconds,27+sizeof(long)," \"secondsWorked\" : %ld }",gsh.intensity);

    if(jsonLonWritten + jsonLatWritten + jsonSecondsWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_heatmapNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif   
    }
    return snprintf(jsonOutput, jsonOutputAllocatedSize-1,"%s%s%s",jsonLat, jsonLon, jsonSeconds);
}


/* Recommed at least 67 + GS_REPORT_MAX_LENGTH*3 + GS_REPORT_TYPE_MAX_LENGTH for safety for jsonOutput size*/
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
    jsonPartial = "\"timestamp\" : \"%s\", \"hash\" : \"%s\",\"type\":\"%s\" }";
    _escapeJSON(gsr.content, GS_REPORT_MAX_LENGTH*3, escaped);

    /* the message content can easily overrun the stack so try to be extra careful */
    snprintf(jsonPartialString,43+65+20+GS_REPORT_TYPE_MAX_LENGTH,jsonPartial,gsr.createdTime, gsr.authorize,gsr.rType);
    snprintf(jsonPartialMsg,strlen(gsr.content)*3+20,jsonMsg, escaped);
    

    return sprintf( jsonOutput,"%s%s", jsonPartialMsg, jsonPartialString);
}

/* 'Safe' gs_reportTOJSON
*/
int gs_reportNToJSON(const struct gs_report gsr, char * jsonOutput, int jsonOutputAllocatedSize){
    #ifndef JSON_CONTENT_LENGTH
        #define JSON_CONTENT_LENGTH (20+(GS_REPORT_MAX_LENGTH)*4+1+GS_REPORT_TYPE_MAX_LENGTH)
    #else
        #undef JSON_CONTENT_LENGTH
        #define JSON_CONTENT_LENGTH (20+(GS_REPORT_MAX_LENGTH)*4+1+GS_REPORT_TYPE_MAX_LENGTH)
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
    jsonHashWritten = snprintf(jsonHash, JSON_HASH_LENGTH," \"hash\" : \"%s\"",gsr.authorize );


    _escapeJSON(gsr.content, strlen(gsr.content), escaped);

    jsonMessageWritten = snprintf(jsonMessage, JSON_CONTENT_LENGTH, "{\"message\" : \"%s\", ",escaped );
    
    if(jsonMessageWritten + jsonHashWritten + jsonTimestampWritten > jsonOutputAllocatedSize + GS_REPORT_TYPE_MAX_LENGTH){
        fprintf(stderr, "%s\n", "gs_reportNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s%s%s,\"type\":\"%s\"}", jsonMessage,jsonTimestamp,jsonHash,gsr.rType);

}


/*Heartbeat Json: { \"heartbeat\" : epoch time}
 * _only_ a safa version available.
*/
 int gs_heartbeatNToJSON(char * jsonOutput, int jsonOutputAllocatedSize){
    char json[strlen("{ \"heartbeat\" : %ld}")+sizeof(long)];
    int jsonWritten;
    time_t heartbeat; 
    bzero(json, strlen("{ \"heartbeat\" : %ld}")+sizeof(long));

    heartbeat = time(0);
    jsonWritten = snprintf(json, strlen("{ \"heartbeat\" : %ld}")+sizeof(long), "{ \"heartbeat\" : %ld}", heartbeat);

    if (jsonWritten > jsonOutputAllocatedSize-1){
        fprintf(stderr, "%s\n", "gs_hearbeatNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif   
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s", json);

 }

/*
{ "id" : 3324523452345, 
  "latDegrees" : 24.53, 
  "lonDegrees" : 43.2, 
   "type" : "COMMENT", 
   "message" : "I need help with the trash on Colchester ave", 
   "addressed" : false }
*/
int gs_markerCommentNToJSON(const struct gs_marker * gsm, const struct gs_comment * gsc ,char * jsonOutput, int jsonOutputAllocatedSize){
    char jsonMarkerId[16]; /*{"id":%ld,*/
    char jsonLat[32]; /*"latDegrees":%s,*/
    char jsonLon[32]; /*"lonDegrees":%s,*/
    char jsonType[32]; /*"type":"%s",*/
    char jsonMessage[32 + (GS_COMMENT_MAX_LENGTH*4)+1]; /*"message":"%s",*/
    char jsonAddressed[32]; /*"addressed":%s}*/
    char latlon[DecimalWidth];
    char escaped[(GS_COMMENT_MAX_LENGTH*4)+1];
    
    int jsonMarkerIdWritten;
    int jsonLatWritten;
    int jsonLonWritten;
    int jsonTypeWritten;
    int jsonMessageWritten;
    int jsonAddressedWritten;

    bzero(jsonMarkerId,sizeof jsonMarkerId);
    bzero(jsonLat,sizeof jsonLat);
    bzero(jsonLon,sizeof jsonLon);
    bzero(jsonType, sizeof jsonType);
    bzero(jsonMessage, sizeof jsonMessage);
    bzero(jsonAddressed, sizeof jsonAddressed);
    bzero(latlon,sizeof latlon);
    bzero(escaped, sizeof escaped);

    jsonMarkerIdWritten = snprintf(jsonMarkerId,sizeof jsonMarkerId, "{\"id\":%ld,", gsm->id);
    formatDecimal(gsm->latitude,latlon);
    jsonLatWritten = snprintf(jsonLat, sizeof jsonLat,"\"latDegrees\":%s,", latlon);
    formatDecimal(gsm->longitude,latlon);
    jsonLonWritten = snprintf(jsonLon, sizeof jsonLat,"\"lonDegrees\":%s,", latlon);
    jsonTypeWritten = snprintf(jsonType, sizeof jsonType,"\"type\":\"%s\",",gsc->cType);
    _escapeJSON(gsc->content, strlen(gsc->content), escaped);
    jsonMessageWritten = snprintf(jsonMessage, sizeof jsonMessage,"\"message\":\"%s\",",escaped);
    jsonAddressedWritten  = snprintf(jsonAddressed, sizeof jsonAddressed,"\"addressed\":%s}",gsm->addressed == ADDRESSED_TRUE ? "true" : "false");

    if(jsonMarkerIdWritten + jsonLatWritten + jsonLonWritten + jsonTypeWritten + jsonMessageWritten + jsonAddressedWritten > jsonOutputAllocatedSize){
        fprintf(stderr, "%s\n", "gs_markerCommentNToJSON may have returned partial JSON output due to not allocating enough memory");
        #ifdef RETURN_ON_JSON_RISK
            RETURN_ON_JSON_RISK;
        #endif
    }
    return snprintf(jsonOutput,jsonOutputAllocatedSize-1, "%s%s%s%s%s%s", jsonMarkerId,jsonLat,jsonLon,jsonType,jsonMessage,jsonAddressed);

}