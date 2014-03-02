#Operating Systems Project 2: HOST Dispatcher

CC = gcc
CCOPTS = -c -g -Wall
LINKOPTS = -g
C99 = -std=gnu99

hostd: main.o memory_mgmt.o process_mgmt.o queue.o util.o
	$(CC) $(LINKOPTS) -o $@ $^
	
util.o: util.c util.c
	$(CC) $(CCOPTS) $(C99) -o $@ $<
	
queue.o: queue.c queue.h util.h
	$(CC) $(CCOPTS) $(C99) -o $@ $<
	
main.o: main.c process_mgmt.h queue.h
	$(CC) $(CCOPTS) $(C99) -o $@ $<
	
memory_mgmt.o: memory_mgmt.c memory_mgmt.h queue.h process_mgmt.h
	$(CC) $(CCOPTS) $(C99) -o $@ $<

process_mgmt.o: process_mgmt.c process_mgmt.h util.h
	$(CC) $(CCOPTS) $(C99) -o $@ $<