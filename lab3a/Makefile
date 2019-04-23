
CC=gcc
CFLAGS = -Wall -Wextra -O3

default:
	$(CC) $(CFLAGS) -o lab3a lab3a.c

clean:
	-rm -f lab3a *.gz

dist:
	tar -czf lab3a-.tar.gz Makefile README ext2_fs.h lab3a.c