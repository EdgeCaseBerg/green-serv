#include "helpers/json.h"
#include <unistd.h>

#define JSON_LENGTH 32
int main(){
	char json[JSON_LENGTH];
	bzero(json, JSON_LENGTH);
	gs_heartbeatNToJSON(json,JSON_LENGTH);
	
	printf("%s\n", json);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}
#undef JSON_LENGTH
