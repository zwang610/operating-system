

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

void cause_sf(){
    char * ptr = NULL;
    *ptr = '?';
}

void handler(){
    fprintf(stderr, "caught segmentation fault");
    exit(4);
}

int main(int argc, char** argv){
    static int seg_flag = 0;
    static int catch_flag = 0;
    static int dump_flag = 0;

    static struct option long_options[] = {
        {"input", required_argument, NULL, 1},
        {"output", required_argument, NULL, 2},
        {"segfault", no_argument, &seg_flag, 1},
        {"catch", no_argument, &catch_flag, 1},
        {"dump-core", no_argument, &dump_flag, 1}, 
        {0, 0, 0, 0}
    };

    int opt_index;
    const char optstring[] = "";
    int c = 0;
    static char* inputf = NULL;
    static char* outputf = NULL;
    
    int ifd; // input file discriptor
    int ofd; // output file discriptor
    
    while (1)
    {
        c = getopt_long(argc, argv, optstring, long_options, &opt_index);
        if (c == -1) break;

        switch (c){     
        case 1:
            inputf = optarg;
            break;
        case 2:
            outputf = optarg;
            break;
        case 0:
            break;

        default:
            fprintf(stderr, "invalid option");
            exit(1);
        }
    }    
    //printf("input: %s, output: %s, segfault: %d, catch: %d, dump-core: %d\n", inputf, outputf, seg_flag, catch_flag, dump_flag);

    if (catch_flag){
        signal(SIGSEGV, &handler);
    }// dump-core can reset catch_flag

    if (seg_flag){
        if (dump_flag){
            exit(139);
        }
        else{
            cause_sf();
            //If segfault is specified, do it immediately, and do not copy from stdin to stdout.
        }
    }

    if (inputf){
        ifd = open(inputf, O_RDONLY); // open read only input file
        if (ifd == -1)
        {
            fprintf(stderr, "--input %s: ", inputf);
            fprintf(stderr, "%s\n", strerror(errno));
            exit(2);
        }
        else 
        {
            close(STDIN_FILENO); // close the file on input stream
            dup(ifd); // put file on input stream
            close(ifd); // cloase file
        }
    }

    if (outputf){
        ofd = creat(outputf, S_IRWXU); // create a filw with read write execute permission, it will also truncate according to the manual
        if (ofd == -1)
        {
            fprintf(stderr, "--output %s: ", outputf);
            fprintf(stderr, "%s\n", strerror(errno));
            exit(3);
        }
        else 
        {
            close(STDOUT_FILENO); 
            dup(ofd); 
            close(ofd);
        }
    }

    const int buf_size = 400; // increase buf_size can reduce the number of system call to improve the performance in some situation
    char buf[buf_size];
    int input_size;
    while ((input_size = read(STDIN_FILENO, &buf, buf_size))> 0){
        write(STDOUT_FILENO, &buf, input_size);
    }   

    exit(0);

}

//--input=filename ... use the specified file as standard input (instead of what is already being used as standard input) 
//If you are unable to open the specified input file, report the failure (on stderr, file descriptor 2) using fprintf(3), and exit(2) with status 1.


//--output=filename ... create the specified file and use it as standard output (instead of whatever is already being used as standard output) 
//If the file already exists, truncate it to zero size. If you are unable to create or truncate the specified output file, report the failure (on stderr, file descriptor 2) using fprintf(3), and exit(2) with status 2.

//--segfault ... force a segmentation fault (e.g., by calling a subroutine that sets a char * pointer to NULL and then stores through the null pointer). If this argument is specified, do it immediately, and do not copy from stdin to stdout.

//--catch ... use signal(2) to register a SIGSEGV handler that catches the segmentation fault, logs an error message (on stderr, file descriptor 2) and exit(2) with status 4.

//--dump-core ... dump core on segmentation faults.