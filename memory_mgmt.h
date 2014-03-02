/*File: memory_mgmt.h
  Description: Contains definitions for memory management-related functions.
  First-fit algorithm is used to determine which memory block to allocate next.
*/

#ifndef MEM_MGMT_H
#define MEM_MGMT_H

#include <stdbool.h>

//Memory allocation block
struct mab {
	int offset;				//Location of memory block	
	int size;				//Size of the memory block		
	bool allocated;			//Indicates whether the block is currently allocated to a process
	struct mab * next;		//Pointer to next block of memory
	struct mab * previous;	//Pointer to previous block of memory
};
typedef struct mab Mab;
typedef Mab * mabptr;

int get_remaining_mem();				   //Returns the amount of free memory available in the system.
bool mem_check(int size);				   //Returns true if memory block of size 'size' can be allocated.
mabptr mem_alloc(int size);				   //Allocates memory block and returns a pointer to it. Returns NULL if allocation failed.
void mem_free(mabptr memory);			   //Frees memory block.
mabptr mem_split(mabptr memory, int size); //Splits block into two. Returns pointer to leftover block.
bool mem_merge(mabptr top, mabptr bottom); /*Merges two blocks. Bottom is combined with top and the bottom pointer is then set to NULL.
										     Returns true if operation was successful. False otherwise.*/

#endif
