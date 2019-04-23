
CC=gcc
CFLAGS=-Wall -Wextra -g
.SILENT:

default:
	$(CC) $(CFLAGS) -o lab0 lab0.c
check: default test1 test2 test3 test4 test5 test6 test7 test8

test1:
# copy from stdin to stdout, finished with exit(0)
	echo | ./lab0 &> /dev/null; if [[ $$? -ne 0 ]]; then echo "test error: copy from stdin to stdout\n"; fi

test2:
# copy from inputfile to stdout, finished with exit(0)
	touch inputf.txt; ./lab0 --input=inputf.txt &> /dev/null; if [[ $$? -ne 0 ]]; then echo "test error: copy from input file to stdout\n"; fi
	rm inputf.txt

test3:
# copy from stdin to outputfile, finished with exit(0)
	echo | ./lab0 --output=outputf.txt; if [[ $$? -ne 0 ]]; then echo "test error: copy from stdin to output file\n"; fi
	rm outputf.txt

test4:
# copy from inputfile to outputfile, finished with exit(0) and files are the same
	touch inputf.txt; ./lab0 --input=inputf.txt --output=outputf.txt; if [[ $$? -ne 0 ]]; then echo "test error: copy from input file to output file\n"; fi
	cmp inputf.txt outputf.txt; if [[ $$? -ne 0 ]]; then echo "test error: input file is different from output file\n"; fi
	rm inputf.txt outputf.txt

test5:
# invalid option test
	echo | ./lab0 --invalid &> /dev/null; if [[ $$? -ne 1 ]]; then echo "test error: cannot recognize invalid option\n"; fi

test6:
# reset catch with dump-core exit(0)
	echo | ./lab0 --segfault --catch --dump-core &> /dev/null; if [[ $$? -ne 139 ]]; then echo "test error: cannot reset catch by dump-core\n"; fi

test7:
# create segfault test and catch segfault, exit(4)
	echo | ./lab0 --segfault --catch &> /dev/null; if [[ $$? -ne 4 ]]; then echo "test error: segfault cannot be caught"; fi 

test8:
# dump-core test exit (139)
	echo | ./lab0 --segfault --dump-core &> /dev/null; if [[ $$? -ne 139 ]]; then echo "test error: cannot dump-core\n"; fi

clean:
	rm -f lab0 lab0-.tar.gz lab0

dist:
	tar -czf lab0-.tar.gz lab0.c Makefile backtrace.png breakpoint.png README