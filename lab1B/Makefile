#!/bin/bash

CC = gcc
CFLAGS=-g -Wall -Wextra
.SILENT:

default:
	$(CC) $(CFLAGS) -o simpsh simpsh.c

check: default 
		@chmod +x test.sh
		@./test.sh

#other cases is tested by the sanity check script provided by TA

clean:
	rm -f simpsh lab1-.tar.gz

dist:
	tar -czf lab1-.tar.gz simpsh.c Makefile README test.sh
