CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -g -Wshadow

all: klient server

klient: klient.c
	$(CC) $(CFLAGS) $^ -o $@

server: server.c
	$(CC) $(CFLAGS) $^ -o $@


runs: server
	./server haiku.job port

runk: klient
		./klient ip port
