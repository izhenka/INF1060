CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -g -Wshadow

all: client server

client: client.c
	$(CC) $(CFLAGS) $^ -o $@

server: server.c
	$(CC) $(CFLAGS) $^ -o $@


runs: server
	./server jobs/haiku.job 5432

runc: client
	./client "127.0.0.1" 5432

runc2: client
	./client "37.191.141.137" 5432


clean:
	rm -f server client
	rm -r client.dSYM server.dSYM
