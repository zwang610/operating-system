

The files included in lab1a- are:
1. simpsh.c: simpsh.c is the source code of simpsh command. User can pass argument and command to create pipes and access files. Since this is only the part A of lab 1. simpsh only has option --rdonly, --wronly, --command, and --verbose. 

2. Makefile: Makefile is a script used to compile, test, packaging files and clean the package

3. README: a brief description of all files, test cases and concerns about the lab

4. test.sh: a short test on the functionality of simpsh, since most of the test cases is included in sanity check script, test.sh only includes three other different cases.

Concerns about the lab1 A:
For me, there is too my confusions to complete this lab. I am not able to get any information about when the program should exit or when the program should continue until I used the sanity script given by TA on Piazza. Though most of my code is correct, my program only passed two cases while it failed the other 13 cases on my first try. I have to make those corrections based on the script step by step. I find out that there is no place mentioning these specific details.

According to the test case provided by TA and questions answered on piazza, partA shouldn't use wait(2). However, without wait(2) command, we are not able to get the exit status of child process, which means we are not able to see if execvp succueed or not. If we have an invalid command used, for example --i o e invalidcmd ..., then it cannot be reported because we cannot get the exit status.  



