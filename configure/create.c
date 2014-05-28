/* configure/create.c
 * Author: Ethan J Eldridge
 * Purpose: This file simply creates a configuration file based on a 
 * 			local file, or makes a default one if none is found
 *
 * Usage:
 *			create a .gs.conf file with name value pairs
 *			make configure
 */
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void startIfNDef(char * macroname){
	printf("#ifndef %s\n", macroname );
}

void elseIt(){
	printf("#else\n");
}

void endIfDev(){
	printf("#endif\n");
}

void defineIt(char * defineName, char * defineBody){
	printf("#define %s %s\n", defineName, defineBody);
}

void unDefIt(char * macroname){
	printf("#undef %s\n", macroname);
}

#define BUFFERSIZE 2048
int main(int argc, char const *argv[]){
	char * confFilePath = ".gs.conf";
	FILE * confFile;
	char * theBuffer;
	char keyBuffer[BUFFERSIZE];
	char valBuffer[BUFFERSIZE];
	ssize_t bytesRead;

	bzero(keyBuffer, sizeof keyBuffer);
	bzero(valBuffer, sizeof valBuffer);

	theBuffer = malloc(BUFFERSIZE);
	if(theBuffer == NULL)
		goto err;

	memset(theBuffer,0,BUFFERSIZE);

	if(argc > 1)
		confFile = fopen(argv[1], "r");
	else
		confFile = fopen(confFilePath, "r");	
		

	if(confFile == NULL)
		goto err;

	/* Read out some data to process */
	
	fclose(confFile);
	return 0;

	closeAndErr:
		fclose(confFile);
	err:
		fprintf(stderr, "%s\n", "There was a problem reading the configuration file.");
		return 1;
}


