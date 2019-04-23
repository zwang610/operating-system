

The files included in lab0- are:
1. lab0.c: 
Lab0.c is the source code of lab0.
The main program read from either input stream or a file and then write it to output stream or a file by choice.
It can also create segmentation fault, catch it and dump core by user's choice.

2. Makefile: 
Makefile is a script used to compile lab0.c, build an executable, make a smoke test, packaging all files and clean the package.

3. breakpoint.png:
breakpoint.png is the screenshot of the occurence of segmentation fault when using breakpoint command to break at the previous step in gdb.

4. backtrace.png:
backtrace.png is the screenshot of the occurence of segmentation fault when using backtrace command in gdb

5. README: a brief description of all files, some description about the smoke test and sources being used in this lab

There are 8 test included in the smoke test part of Makefile:
1. test if the program is able to copy from stdin to stdout
2. test if the program is able to copy from a file to stdout
3. test if the program is able to copy from stdin to a file
4. test if the program is able to copy from one file to another file
5. test if the program is able to recognize an invalid option and exit 1
6. test if the program is able to use dump-core to reset catch
7. test if the program is able to catch a segfault
8. test if the program is able to dump-core

Source being used:
To avoid invaluable output in terminal, I used .SILENT in Makefile which I found in https://stackoverflow.com/questions/24005166/gnu-make-silent-by-default/24011502
I also used http://man7.org/linux/man-pages/ to searched about how to use multiple command such as optget(3), create(2), open(2) and etc.
