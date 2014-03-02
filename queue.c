/***********************************************
 * File: queue.c
 * Description: Queue data structure.
***********************************************/

#include<stdlib.h>
#include<stdio.h>
#include "queue.h"

#define DEBUG_QUEUE false //Debug flag specific to this file

//Initializes an empty queue
void init_queue(queue* q) {
	q->front = NULL;
	q->back = NULL;
	q->timer = 0;
}

/*Adds the PCB into the queue*/
void enqueue(queue* q, pcbptr value) {
	value->next = NULL;
	if(!isEmptyQueue(*q) )
		q->back->next = value;
	else
		q->front = value;
	q->back = value;
}

//Removes element at the front of the queue.
pcbptr dequeue(queue* q) {
	if(q == NULL)
		return NULL;	
		
	pcbptr front = q->front;
	if (front != NULL) {
		q->front = q->front->next;
	}
	return front;
}

//Returns true if queue is empty. False otherwise
bool isEmptyQueue(queue q) {
	return q.front == NULL;
}
