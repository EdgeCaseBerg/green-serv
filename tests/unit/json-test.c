#include <stdlib.h>
#include "helpers/json.h"
#include "helpers/decimal.h"
#include "helpers/strmap.h"
#include <unistd.h>

#define TEST_1 "{\n\"type\":\"COMMENT\",\n\"message\":\"Test\"\n}"
#define TEST_2 "{\"type\":\"COMMENT\",\"message\":\"Test\"}"
#define TEST_3 "{}"
#define TEST_4 "{\"key\":12,\"something\":\"something-value\"}"
#define TEST_5 "{   \"key\" : \"valuue\" }"

int main() {
	
	StrMap * sm = NULL;

	sm = sm_new(15);
	if(sm == NULL){
		goto out_of_mem;
	}


	parseJSON(TEST_1, strlen(TEST_1), sm);
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "type"));
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "message"));
	sm_delete(sm);
	sm = sm_new(15);
	if(sm == NULL){
		goto out_of_mem;
	}

	parseJSON(TEST_2, strlen(TEST_2), sm);
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "type"));
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "message"));
	sm_delete(sm);
	sm = sm_new(15);
	if(sm == NULL){
		goto out_of_mem;
	}

	parseJSON(TEST_3, strlen(TEST_3), sm);
	fprintf(stderr, "Number Keys: %d/0\n", sm_get_count(sm));
	sm_delete(sm);
	sm = sm_new(15);
	if(sm == NULL){
		goto out_of_mem;
	}
	parseJSON(TEST_4, strlen(TEST_4), sm);
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "key"));
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "something"));
	sm_delete(sm);
	sm = sm_new(15);
	if(sm == NULL){
		goto out_of_mem;
	}
	parseJSON(TEST_5, strlen(TEST_5), sm);
	fprintf(stderr, "Exists: %d\n", sm_exists(sm, "key"));

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	sm_delete(sm);
	return 0;

	out_of_mem:
		fprintf(stderr, "Couldn't do test out of memory\n");
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return 1;
}