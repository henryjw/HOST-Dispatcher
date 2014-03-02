/************************************************************
 *File: util.h
 *Description: Contains declarations misc. helper functions.
 ***********************************************************/

#ifndef UTIL_H
#define UTIL_H

#define DEBUG false
#define NEWLINE '\n'

int arraySize(const void** array); 				//Returns number of elements in NULL-terminated array
int * readInfo(FILE *file); 		  				/*Reads a line in format "<int1>, <int2>, ..., <int8>" from a filestream and returns the int values in an
															  malloced array of size 8.*/
void read_file(FILE* file, char* buffer, size_t size); //Reads the contents of a file and stores them in a string.

#endif
