#include "helpers/json.h"

#define JSON_LENGTH 32
int main(){
	char json[JSON_LENGTH];
	bzero(json, JSON_LENGTH);
	gs_heartbeatNToJSON(json,JSON_LENGTH);
	
	printf("%s\n", json);

	return 0;
}
#undef JSON_LENGTH
