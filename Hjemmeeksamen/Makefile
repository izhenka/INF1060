CC=gcc
CFLAGS= -Wall -Wextra -std=c99 -Wshadow

all: klient.c server.c
	$(CC) $(CFLAGS) $^ -o $@

run: ./all ip port
