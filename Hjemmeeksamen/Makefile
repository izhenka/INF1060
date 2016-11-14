CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -g -Wshadow

all: client server

client: client.c
	$(CC) $(CFLAGS) $^ -o $@

server: server.c
	$(CC) $(CFLAGS) $^ -o $@


runs: server
	./server jobs/haiku.job port

runc: klient
	./klient ip port


clean:
	rm -f server client
