#include "network/router.h"

static inline int min(int a, int b){
	return a < b ? a : b;
}

int determineController(char * url){
	/* Compare only the length up to the point where parameters might
	 * show up. This is not a regex after all. (although we could prob
	 *-ably use strstr...)
	*/

	if(strncasecmp(url, "/api/comments",13 ) == 0		||
			strncasecmp(url,"/api/comments/",14 ) == 0	){
			/* Comments Controller */
			return 	COMMENTS_CONTROLLER;
	}else if(strncasecmp(url, "/api/heatmap",12 ) == 0	||
			strncasecmp(url, "/api/heatmap/",13 ) == 0	){
			return HEATMAP_CONTROLLER;
	}else if(strncasecmp(url,"/api/pins",9 ) == 0		||
			strncasecmp(url,"/api/pins/",10 ) == 0		){
			return MARKER_CONTROLLER;
	}else if(strncasecmp(url,"/api/debug",10 ) == 0 	||
			strncasecmp(url,"/api/debug/",11 ) == 0		){
			return REPORT_CONTROLLER;
	}else if(strcasecmp(url, "") == 0 		||
			strcasecmp(url, "/" ) == 0		||
			strcasecmp(url, "/api/" ) == 0	||
			strcasecmp(url, "/api" ) == 0	){
			/* HeartBeat Controller */
			return HEARTBEAT_CONTROLLER;
	}/* No Route matches. */
	return INVALID_CONTROLLER;
}

/*
 * Parse the url and store values into the hash table
 * returns the number of values successfully placed into the hashtable
 * all key values are LOWERCASED upon return
*/
int parseURL(const char * url, int urlLength, StrMap * table){
    int i;
    int j;
    int pairCounter;
    char keyBuff[256];
    char valBuff[256];
    bzero(keyBuff,sizeof keyBuff);
    bzero(valBuff,sizeof valBuff);

    if(url == NULL)
        return 0;
    if(table == NULL)
        return -1; /* Err ... */
    pairCounter = 0;
    /* Find ? */
    for(i=0; url[i] != '\0' && i < urlLength; ++i)
        if(url[i] == '?')
            break;
    while(url[i] != '\0' && i < urlLength){
        for(j=0,i++; url[i] != '=' && url[i] != '\0' && i < urlLength; ++i)
            keyBuff[j++] = (int)url[i] > 64 && url[i] < 91 ? url[i] + 32 : url[i];
            /* Store the case as all LOWERCASE */
        keyBuff[j] = '\0';
        for(j=0,i++; strlen(keyBuff) > 0 && url[i] != '&' && url[i] != '\0' && i < urlLength; ++i)
            valBuff[j++] = url[i];
        valBuff[j] = '\0';
        if(strlen(valBuff) > 0 && strlen(keyBuff) > 0){
            /* Place values into table */
            if(sm_put(table, keyBuff, valBuff) == 0)
                fprintf(stderr, "Failed to copy parameters into hash table while parsing url\n");
            else
                pairCounter++;
        }
        /* reset the buffers */
        bzero(keyBuff,sizeof keyBuff);
        bzero(valBuff,sizeof valBuff);
        
    }
    return pairCounter;
}
