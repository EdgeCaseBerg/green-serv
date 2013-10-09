#include "network/net.h"
#include "helpers/strmap.h"
#include <stdlib.h>
#include <stdio.h>

void testURL(char * url, StrMap * sm, int passed){
	int params;

	params = parseURL(url, strlen(url), sm);
	printf("Expected %d params, got: %d\n",passed, params );
}

int main(){
	StrMap * sm;
	char buff[200];
	bzero(buff, sizeof buff);

	sm = sm_new(HASH_TABLE_CAPACITY);
	if (sm == NULL) {
		fprintf(stderr, "Cannot allocate space for hash table\n");
		return 1;
	}

	testURL("http://www.bob.com/", sm, 0);

	testURL("http://www.bob.com/?", sm, 0);

	testURL("http://www.bob.com/?incomplete=", sm, 0);
	if(sm_exists(sm, "incomplete") == 1)
		fprintf(stderr, "Did not parse incomplete val pair right\n");

	testURL("http://www.bob.com/?valid=true", sm, 1);
	if(sm_exists(sm, "valid") == 0)
		fprintf(stderr, "Did not parse valid val pair right\n");	

	testURL("http://www.bob.com/?one=truth&two=love&three=door",sm,3);
	if(sm_exists(sm, "one") == 0)
		fprintf(stderr, "Did not parse incomplete val pair right\n");
	
	sm_get(sm, "one", buff, sizeof buff);
	printf("%s\n", buff);
	sm_delete(sm);

	return 0;
}