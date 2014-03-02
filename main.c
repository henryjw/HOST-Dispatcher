/**********************************************************
 *OS Project 2: HOST Dispatcher
 *File:main.c
 *Author: Henry Williams
 *Written using Notepad++ using tab size of 4.
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "process_mgmt.h"
#include "memory_mgmt.h"
#include "queue.h"

int main(int argc, char* argv[]) {
	char * fileName = NULL;
	/* If an argument (file name) is passed, then set the file name. Otherwise,
	   display contents of the readme file and exit.*/
	if(argc > 1)
		fileName = argv[1];
	else {
		unsigned int SIZE = 2048;
		char* buffer = malloc(SIZE);
		FILE *readme = fopen("readme.txt", "r");
		if(readme == NULL) {
			printf("ERROR - Could not open readme file\n");
			exit(EXIT_FAILURE);
		}
		read_file(readme, buffer, SIZE);
		printf("%s", buffer);

		fclose(readme);
		return 0;
	}
	FILE * file = fopen(fileName, "r");

	if (file == NULL) {
		printf("ERROR - Could not open file \"%s\"\n", fileName);
		exit(EXIT_FAILURE);
	}

	init_dispatcher(file);
	fclose(file);
	start_dispatcher(); //Run the dispatcher.
	return 0;
}
