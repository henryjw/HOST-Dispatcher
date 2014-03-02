/*********************************************************
 * File: process_mgmt.h
 * Description: Contains functions to manage processes
 *********************************************************/
 
#ifndef PROCESS_MGMT_H
#define PROCESS_MGMT_H

#include "queue.h"
#include "util.h"

/***************Total Resources***************/
#define  TOTAL_MEM 	  1024	//Total amount of memory (in MB) for processes
#define	 RESERVED_MEM 64		//Amount of memory (in MB) reserved for real-time processes
#define  NUM_PRINTERS 2			//Number of printers
#define  NUM_SCANNERS 1			//Number of scanners
#define  NUM_MODEMS	 1			//Number of modems
#define  NUM_CDS	 2				//Number of CD drives
/********************************************/

enum indices{arrival_time, priority, cpu_time, memory_alloc, num_printers,
					num_scanners, num_modems, num_cds};
					
pcbptr create_pcb();											//Returns a pointer to an empty PCB.
bool areEmptyQueues();											//Returns true if all dispatcher queues are empty.
bool rsrc_chk(int printers, int scanners, int modems, int cds); //Returns true if the requested resources are available.
bool rsrc_alloc (pcbptr process, int printers,					//Allocates resources to process. Returns true if resources were allocated. False otherwise.
					int scanners, int modems, int cds);			
void start_process(pcbptr process);								//Starts process. 
void restart_process(pcbptr process);							//Restarts suspended process.
void suspend_process(pcbptr process);							//Suspends process
void kill_process(pcbptr process); 								//Terminates process and frees its resoures.
void init_dispatcher(FILE *file);								//Initializes dispatchers
void start_dispatcher();											//Starts the process dispatcher
void init_process(pcbptr process, int * processInfo);		//Initializes process block
void placeInQueue(pcbptr process);								//Adds process to appropriate queue based on its priority level.


#endif
