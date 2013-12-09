#include "controllers/heartbeat.h"
#include <unistd.h>

#define JSON_LENGTH 32
int main(){
	char json[JSON_LENGTH];
	bzero(json, JSON_LENGTH);
	heartbeat_get(json,JSON_LENGTH);
	
	printf("%s\n", json);

	/* Try it out with a not-big-enough buffer*/
	heartbeat_get(json,2);
	printf("%s\n", json);

	/* Try it with too big a buffer --expect nothin*/
	heartbeat_get(json,JSON_LENGTH*2);
	printf("%s\n", json);

	/* It would be good to make some kind of JSON validifier */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}
#undef JSON_LENGTH
