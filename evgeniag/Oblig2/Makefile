
CC=gcc
CFLAGS=-Wall -Wextra -std=c99


all: main.c router.c
	$(CC) $(CFLAGS) $^ -o $@

run1:
		./all 1router.dat

run5:
		./all 5router.dat

run10:
		./all 10router.dat

run100:
		./all 100router.dat

clean:
	rm -f router main
