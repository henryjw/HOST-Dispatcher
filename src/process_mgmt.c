/*********************************************************
 * File: process_mgmt.c
 * Description: Defines functions to manage processes
 *********************************************************/
 
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //For process management (execvp, kill, etc.)
#include "../inc/process_mgmt.h"
#include "../inc/memory_mgmt.h"
#include "../inc/queue.h"

#define DEBUG_PROCESS false						//Debug flag specific to this file

const int mQUANTUM = 1;							//CPU time allocated to each process in the feedback queue.
const int mTotalMem = TOTAL_MEM - RESERVED_MEM; //Total memory in the system (excluding the memory reserved for real-time processes).
int mDispatcher_timer;
char *mProcessName[] = {"./process", NULL};		//Process parameters (for execvp())
int status = 0;									//Status passed to waitpid

int mPrinters = NUM_PRINTERS;					//Number of free printers
int mScanners = NUM_SCANNERS;					//Number of free scanners
int mModems = NUM_MODEMS;						//Number of free modems
int mCDs = NUM_CDS;								//Number of free CD drives

queue mInput;	//Input queue
queue mJobs;	//User job queue

queue mFcfs;	/*First-come-first-serve queue used for real time (priority = 0) processes
				  This queue must be empty before the other queues are activated.*/
queue mLevel1;	//High priority feedback queue (priority = 1)
queue mLevel2;	//Medium priority feedback queue (priority = 2)
queue mLevel3;  //Low priority feedback queue (priority = 3)

pcbptr mActiveProcess = NULL;	//Currently active process. Set to NULL after running process is terminated.

mabptr mReservedMem = NULL;		//Memory reserved for real-time processes.


//Frees data (from actual memory) that the process holds.
void free_process_pointers (pcbptr process) {
	free(mActiveProcess->info);
	free(mActiveProcess);
}

//Free resources allocated to a process
void rsrc_free(pcbptr process) {
	mPrinters += process->info[num_printers];
	mScanners += process->info[num_scanners];
	mModems += process->info[num_modems];
	mCDs += process->info[num_cds];

	process->info[num_printers] = 0;
	process->info[num_scanners] = 0;
	process->info[num_modems] = 0;
	process->info[num_cds] = 0;

	if (DEBUG_PROCESS) {
		/*Assertion. Theoretically, this should never execute.*/
		if (mPrinters > NUM_PRINTERS || mScanners > NUM_SCANNERS ||
				mModems > NUM_MODEMS || mCDs > NUM_CDS) {
			printf("An error occurred in rsrc_free()\n");
			exit(EXIT_FAILURE);
		}
	}
}

//Starts the process dispatcher
void start_dispatcher() {
	do {
		mabptr allocatedMem = NULL; //Pointer to memory that will be allocated to a process.

		/*Unload pending processes from the input queue*/
		while (mInput.front != NULL && mInput.front->arrival_time <= mDispatcher_timer) {
			if(DEBUG)
				printf("New process added to system\n");

			/*Remove process from input queue and add it to the user job queue if it is a user job.
			  Otherwise, add it to the real-time processes queue.*/
			pcbptr newProcess = dequeue(&mInput);
			/*If the process is larger than the total available memory, it is not admitted into the system*/
			if(newProcess->priority != 0) {
				/*If job requires more memory than the system has in total, display appropriate message and
				  do not admit the process.*/
				if (newProcess->info[memory_alloc] > mTotalMem) {
					printf("\nERROR - Job memory request(%dMB) exceeds total memory(%dMB) - job deleted\n\n",
							newProcess->info[memory_alloc], mTotalMem);
				} else if (newProcess->info[num_printers] > NUM_PRINTERS ||
							newProcess->info[num_scanners] > NUM_SCANNERS ||
							newProcess->info[num_modems] > NUM_MODEMS ||
							newProcess->info[num_cds] > NUM_CDS) {
					printf("\nERROR - Job demands too many resources - job deleted\n\n");
				}
				else {
					enqueue(&mJobs, newProcess);
				}
			}
			else {
				/*If job requires more memory than the system has in total, display appropriate message and
				  do not admit the process.*/
				if (newProcess->info[memory_alloc] > RESERVED_MEM) {
					printf("\nERROR - Real-time memory request(%dMB) exceeds reserved memory(%dMB) - job deleted\n\n",
							newProcess->info[memory_alloc], RESERVED_MEM);
				} else if (newProcess->info[num_printers] > 0 ||
						newProcess->info[num_scanners] > 0 ||
						newProcess->info[num_modems] > 0 ||
						newProcess->info[num_cds] > 0) {
					printf("\nERROR - Real-time job not allowed I/O resources - job deleted\n\n");
				}
				else
					enqueue(&mFcfs, newProcess);
			}
		}

		/*Unload pending processes from user jobs queue while the memory can be allocated to them.*/
		while(mJobs.front != NULL &&
				mem_check(mJobs.front->info[memory_alloc]) &&
				 rsrc_chk(mJobs.front->info[num_printers], mJobs.front->info[num_scanners],
						  mJobs.front->info[num_modems], mJobs.front->info[num_cds])) {
			/*Allocate memory to process and place it in appropriate queue based on its priority level.*/
			allocatedMem = mem_alloc( mJobs.front->info[memory_alloc]);
			pcbptr process = dequeue(&mJobs);
			process->memory = allocatedMem;
			rsrc_alloc(process, process->info[num_printers], process->info[num_scanners],
					process->info[num_modems], process->info[num_cds]);
			placeInQueue(process);
		}

		/*If a process is running*/
		if (mActiveProcess != NULL) {
			mActiveProcess->remaining_cpu_time--;
			/*If process is done executing, terminate it and free its resources.*/
			if(mActiveProcess->remaining_cpu_time == 0) {
				kill_process(mActiveProcess);
				free_process_pointers(mActiveProcess);
				mActiveProcess = NULL;		//Set active process to NULL to indicate there is no currently running process
			} 
			else if(mActiveProcess->priority > 0 && !areEmptyQueues()) {
				/*If active process is not a real-time process and there are processes in other queues, 
				  then suspend the active process*/
				suspend_process(mActiveProcess);
				/*If the process's priority is < 3, reduce its priority (higher # = lower priority)
				  and send it to its appropriate queue. */
				if(mActiveProcess->priority < 3)
					mActiveProcess->priority++;
				placeInQueue(mActiveProcess);
				mActiveProcess = NULL;	//Set active process to NULL to indicate there is no currently running process
			}
		}	//End if
		if(mActiveProcess == NULL && !areEmptyQueues()) {
			/*Run the a process from the highest priority queue that isn't empty*/
			if (!isEmptyQueue(mFcfs))
				mActiveProcess = dequeue(&mFcfs);
			else if (!isEmptyQueue(mLevel1))
				mActiveProcess = dequeue(&mLevel1);
			else if (!isEmptyQueue(mLevel2))
				mActiveProcess = dequeue(&mLevel2);
			else if (!isEmptyQueue(mLevel3))
				mActiveProcess = dequeue(&mLevel3);

			if(mActiveProcess->priority == 0)
				mActiveProcess->memory = mReservedMem;

			/*If process is suspended, restart it. Otherwise, start it.*/
			if(mActiveProcess->status == SUSPENDED)
				restart_process(mActiveProcess);
			else
				start_process(mActiveProcess);
		} //End if		
		sleep(mQUANTUM);
		mDispatcher_timer += mQUANTUM;
	} while (!areEmptyQueues() || mActiveProcess != NULL ||		/*Loop continues until all queues are empty and there is no process running*/
				!isEmptyQueue(mJobs) || !isEmptyQueue(mInput));	//End while
}

//Returns true if all queues (excluding input queue) are empty)
bool areEmptyQueues() {
	return isEmptyQueue(mFcfs) && isEmptyQueue(mLevel1) &&
		isEmptyQueue(mLevel2) && isEmptyQueue(mLevel3);
}

//Returns a pointer to an empty PCB
pcbptr create_pcb() {
	pcbptr control_block = malloc(sizeof(pcb));
	control_block->pid = 0;
	control_block->arrival_time = 0;
	control_block->remaining_cpu_time = 0;
	control_block->next = NULL;
	
	return control_block;
}
 
//Starts process
void start_process(pcbptr process) {
	int pid = fork();	
	if(pid < 0)										//Error
		printf("\tError creating process\n");
	else if (pid == 0) {							//Child executing
		execvp(process->args[0], process->args);
		printf("\tError executing process \"%s\"   %d\n", process->args[0], getpid());
	}
	else {											//Parent executing
		process->pid = pid;
		process->status = RUNNING;
		if(DEBUG)
			printf("\tProcess %d started.\n", process->pid);

		int * info = process->info;
		/*Display process info*/
		printf("    pid\t    arrive\tprior\tcpu\toffset\tMBytes\tprn\tscn\tmodem\tcd\tstatus\n");
		printf("  %d\t    %d\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
			process->pid, info[arrival_time], info[priority], info[cpu_time], process->memory->offset,
			info[memory_alloc], info[num_printers], info[num_scanners], info[num_modems],
			info[num_cds], "RUNNING");
	}	
}

//Restarts process
void restart_process(pcbptr process) {
	if(kill(process->pid, SIGCONT) != 0)
		printf("\tTerminate of %d failed.\n", process->pid);
	else if(DEBUG)
		printf("\tProcess %d restarted.\n", process->pid);
	process->status = RUNNING;
}

//Suspends process
void suspend_process(pcbptr process) {
	if(kill(process->pid, SIGTSTP) != 0) {
		printf("Suspend of %d failed.\n", process->pid);
		return;
	} 
	if(DEBUG)
		printf("Process %d suspended.\n", process->pid);		
	
	waitpid(process->pid, &status, WUNTRACED); //Wait for process to be stopped before continuing execution
	process->status = SUSPENDED; 
}

///Terminates process and frees its resources
void kill_process(pcbptr process) {
	if(kill(process->pid, SIGINT) != 0) {
		printf("Terminate of %d failed.\n", process->pid);
		return;
	}
	else if(DEBUG)
		printf("Process %d terminated.\n", process->pid);
		
	waitpid(process->pid, &status, WUNTRACED); //Wait for process to be terminated before continuing execution

	/*Free the resources for user job*/
	if(process->priority != 0) {
		mem_free(process->memory);
		rsrc_free(process);
	}
	process->status = TERMINATED;
}

//Initializes process block
void init_process(pcbptr process, int * processInfo) {
	process->args = mProcessName;
	process->arrival_time = processInfo[arrival_time];
	process->remaining_cpu_time = processInfo[cpu_time];
	process->priority = processInfo[priority];
	process->info = processInfo;
}

//Initializes dispatcher
void init_dispatcher(FILE *file) {
	if(mReservedMem == NULL) {
		mReservedMem = mem_alloc(RESERVED_MEM);
		mReservedMem->allocated = true;
	}

	mDispatcher_timer = 0;
	mActiveProcess = NULL;
	/*Initialize queues*/
	init_queue(&mInput);
	init_queue(&mJobs);
	init_queue(&mFcfs);
	init_queue(&mLevel1);
	init_queue(&mLevel2);
	init_queue(&mLevel3);

	/*Populate input queue.*/
	while (!feof(file)) {
		int *processInfo = readInfo(file);	
		if (processInfo == NULL)
			continue;			
		pcbptr process = create_pcb();
		init_process(process, processInfo);		
		enqueue(&mInput, process);
	}
}

//Adds process to appropriate queue based on its priority level.
void placeInQueue(pcbptr process) {
	switch(process->priority) {
		case 0:
			enqueue(&mFcfs, process);
			break;
		case 1:
			enqueue(&mLevel1, process);
			break;
		case 2:
			enqueue(&mLevel2, process);
			break;
		case 3:
			enqueue(&mLevel3, process);
			break;
		default:
			if(DEBUG)
				printf("Invalid input in placeInQueue()\n");
			break;			
	}			
}

//Returns true if the requested resources are available.
bool rsrc_chk(int printers, int scanners, int modems, int cds) {
	return printers <= mPrinters && scanners <= mScanners &&
			modems <= mModems && cds <= mCDs;
}

//Allocates resources to process. Returns true if resources were allocated. False otherwise.
bool rsrc_alloc (pcbptr process, int printers, int scanners, int modems, int cds) {
	if (!rsrc_chk(printers, scanners, modems, cds))
		return false;

	/*Allocate resources*/
	process->info[num_printers] = printers;
	process->info[num_scanners] = scanners;
	process->info[num_modems] = modems;
	process->info[num_cds] = cds;

	/*Update resource counts*/
	mPrinters -= printers;
	mScanners -= scanners;
	mModems -= modems;
	mCDs -= cds;

	return true;
}
