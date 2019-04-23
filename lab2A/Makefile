CC = gcc
CFLAGS= -std=gnu11 -pthread -Wall -Wextra

default:
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c

tests: default 
	   @chmod +x test.sh
	   @./test.sh

graphs: @./lab2_add.gp
		@./lab2_list.gp

dist: 
	tar -czf lab2a-.tar.gz *.c Makefile README test.sh *.csv *.h *.png

clean:
	rm -f lab2_add lab2_list *.png *.csv 