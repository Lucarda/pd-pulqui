CC = gcc
CFLAGS  = -Wall 

install: all

all: pq

pq: pq.o
	$(CC) $(CFLAGS) -O3 -o pq pq.o -Wl,-s

pq.o: pq.c 
	$(CC) $(CFLAGS) -c pq.c 
	
clean: 
	$(RM) pq *.o
