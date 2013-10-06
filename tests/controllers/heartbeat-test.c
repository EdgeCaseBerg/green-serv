#include "controllers/heartbeat.h"

#define JSON_LENGTH 32
int main(){
	char json[JSON_LENGTH];
	bzero(json, JSON_LENGTH);
	heartbeat_get(json,JSON_LENGTH);
	
	printf("%s\n", json);

	return 0;
}
#undef JSON_LENGTH
