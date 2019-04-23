

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/wait.h>
#include <ctype.h>

int main (int argc, char** argv){
    int vb_flag = 0;

    struct option long_options[] = {
        {"rdonly", required_argument, NULL, 'r'},
        {"wronly", required_argument, NULL, 'w'},
        {"command", required_argument, NULL, 'c'},
        {"verbose", no_argument, &vb_flag, 1},
        {0,0,0,0}
    };

    int exit_stat = 0;
    int fd_count = -1;
    int fd_arr[200];
    char* rd_arg;
    char* wr_arg;
    int i,o,e,j,k,arg_begin; // variable used for --command, j is the number of argument follow by cmd
    int opt_index;
    const char optstring[] = "";

    while (1)
    {
        int c = getopt_long(argc, argv, optstring, long_options, &opt_index);
        if (c == -1) break;

        switch(c){
        case 'r': //rdonly
            fd_count++;
            rd_arg = optarg;
            fd_arr[fd_count] = open(rd_arg, O_RDONLY);
            if (fd_arr[fd_count] < 0) {
                fprintf(stderr, "cannot open readonly file\n");
                exit_stat = 1;
            }
            if (vb_flag) fprintf(stdout, "--rdonly %s ", rd_arg);
            break;
        case 'w': //wronly
            fd_count++;
            wr_arg = optarg;
            fd_arr[fd_count] = open(wr_arg, O_WRONLY);
            if (fd_arr[fd_count] < 0) {
                fprintf(stderr, "cannot open writeonly file\n");
                exit_stat = 1;
            }
            if (vb_flag) fprintf(stdout, "--wronly %s ", wr_arg);
            break;
        case 'c': //command
            arg_begin = optind - 1;
            if (!isdigit(argv[arg_begin][0]) || !isdigit(argv[arg_begin+1][0]) || !isdigit(argv[arg_begin+2][0]) ){
                fprintf(stderr, "invalid file descriptor");
                exit(1);
            }
            i = atoi(argv[arg_begin]);
            o = atoi(argv[arg_begin+1]);
            e = atoi(argv[arg_begin+2]); 
            if (i<0||o<0||e<0||i>fd_count||o>fd_count||e>fd_count){
                fprintf(stderr, "invalid file descriptor");
                exit(1);
            }
            for ( j = 0; arg_begin+3+j < argc; j++){
                if (((argv[arg_begin+3+j][0] == '-') 
                && (argv[arg_begin+3+j][1] == '-'))
                ||(argv[arg_begin+3+j] == NULL)) break;
            } 
            char** options = malloc(sizeof(char*)*(j+1));
            for (k = 0; k < j; k++){
                options[k] = argv[arg_begin+3+k];
            }
            options[j] = NULL;
            if (vb_flag){
                fprintf(stdout, "--command %d %d %d ", i, o, e);
                for (k = 0; k < j; k++) {
                    fprintf(stdout, "%s ", options[k]);
                }
            }

            optind = optind + 2 + j;

            pid_t pid = fork();
            if (pid < 0){
                fprintf(stderr, "cannot fork\n");
            }
            if (pid==0){//pid = 0, child process
                dup2(fd_arr[i],0);
                dup2(fd_arr[o],1);
                dup2(fd_arr[e],2);
                close(fd_arr[i]);
                close(fd_arr[o]);
                close(fd_arr[e]);
                int xct = execvp(options[0],options);
                if (xct<0) {
                    fprintf(stderr, "cannot execute the command\n"); 
                    exit(1);
                }
                else{
                    exit(0);
                }
            }

            free(options);

            /*int pidstat;
            if (waitpid(pid, &pidstat, 0) == -1) {
                fprintf(stderr, "waitpid fail");
                exit(1);
            }

            if (pidstat != 0) exit(1);
            */

            break;
        case 0: // all no_argment:verbose
            break;
        
        default:
            fprintf(stderr, "invalid option\n");
            exit_stat = 1;
        }
    }
    
    exit(exit_stat);
}