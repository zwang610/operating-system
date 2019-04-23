
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
void handler(int s){
    fprintf(stderr, "error %d caught", s);
    exit(s);
}

int main (int argc, char** argv){
    int vb_flag;
    int cfd;
    struct option long_options[] = {
        // file-flag
        {"append", no_argument, NULL, 1},
        {"cloexec", no_argument, NULL, 2},
        {"creat", no_argument, NULL, 3},
        {"directory", no_argument, NULL, 4},
        {"dsync", no_argument, NULL, 5},
        {"excl", no_argument, NULL, 6},
        {"nofollow", no_argument, NULL, 7},
        {"nonblock", no_argument, NULL, 8},
        {"rsync", no_argument, NULL, 9},
        {"sync", no_argument, NULL, 10},
        {"trunc", no_argument, NULL, 11},
        // file-open
        {"rdonly", required_argument, NULL, 'r'},
        {"wronly", required_argument, NULL, 'w'},
        {"rdwr", required_argument, NULL, 'x'},
        {"pipe", no_argument, NULL, 'p'},
        // subcommand
        {"command", required_argument, NULL, 'c'},
        {"wait", no_argument, NULL, 'a'},
        // miscellaneous
        {"verbose", no_argument, NULL, 12},
        {"close", required_argument, NULL, 'l'},
        {"abort", no_argument, NULL, 'b'},
        {"catch", required_argument, NULL, 'h'},
        {"ignore", required_argument, NULL, 'i'},
        {"default", required_argument, NULL, 'd'},
        {"pause", no_argument, NULL, 'u'},
        {0,0,0,0}
    };

    int catch_arg;
    int ignore_arg;
    int default_arg;
    int file_flag = 0;
    int exit_stat = 0;
    int fd_count = -1;
    int prc_count = 0;
    char** options = malloc(sizeof(char*)*50);
    int fd_arr[50];
    int fd_vld[50];
    pid_t pid_arr[50];
    char* options_arr[50];
    int i,o,e,j,k,arg_begin; // variable used for --command, j is the number of argument follow by cmd
    int l;
    int opt_index;
    const char optstring[] = "";

    while (1)
    {
        int c = getopt_long(argc, argv, optstring, long_options, &opt_index);
        if (c == -1) break;
        switch(c){
    // file flag, dont know if need to print when verbose flag on
        case 1:
            file_flag = file_flag | O_APPEND;
            break;
        case 2:
            file_flag = file_flag | O_CLOEXEC;
            break;
        case 3:
            file_flag = file_flag | O_CREAT;
            break;
        case 4:
            file_flag = file_flag | O_DIRECTORY;
            break;
        case 5:
            file_flag = file_flag | O_DSYNC;
            break;
        case 6:
            file_flag = file_flag | O_EXCL;
            break;
        case 7:
            file_flag = file_flag | O_NOFOLLOW;
            break;
        case 8:
            file_flag = file_flag | O_NONBLOCK;
            break;
        case 9:
            file_flag = file_flag | O_RSYNC;
            break;
        case 10:
            file_flag = file_flag | O_SYNC;
            break;
        case 11:
            file_flag = file_flag | O_TRUNC;
            break;
        case 12:
            vb_flag = 1;
            break;
    // file open
        case 'r': //rdonly
            fd_count++;
            char* rd_arg = optarg;
            fd_arr[fd_count] = open(rd_arg, file_flag | O_RDONLY, 0644);
            fd_vld[fd_count] = 1;
            //printf("\n open file, fcount: %d, fd: %d", fd_count, fd_arr[fd_count]);
            file_flag = 0;
            if (fd_arr[fd_count] < 0) {
                fprintf(stderr, "cannot open readonly file %d\n", fd_count);
                if (exit_stat == 0) exit_stat = 1;
            }
            if (vb_flag) {
                fprintf(stdout, "--rdonly %s \n", rd_arg);
                fflush(stdout); 
            }
            break;
        case 'w': //wronly
            fd_count++;
            char* wr_arg = optarg;
            fd_arr[fd_count] = open(wr_arg, file_flag | O_WRONLY, 0644);
            fd_vld[fd_count] = 1;
            //printf("\n open file, fcount: %d, fd: %d", fd_count, fd_arr[fd_count]);
            file_flag = 0;
            if (fd_arr[fd_count] < 0) {
                fprintf(stderr, "cannot open writeonly file %d\n", fd_count);
                if (exit_stat == 0) exit_stat = 1;
            }
            if (vb_flag) {
                fprintf(stdout, "--wronly %s \n", wr_arg);
                fflush(stdout); 
            }
            break;
        case 'x'://rdwr
            fd_count++;
            char* rdwr_arg = optarg;
            fd_arr[fd_count] = open(rdwr_arg, file_flag | O_RDWR, 0644);
            fd_vld[fd_count] = 1;
            //printf("\n open file, fcount: %d, fd: %d", fd_count, fd_arr[fd_count]);
            file_flag = 0;
            if (fd_arr[fd_count] < 0) {
                fprintf(stderr, "cannot open file for read and write file %d\n", fd_count);
                if (exit_stat == 0) exit_stat = 1;
            }
            if (vb_flag) {
                fprintf(stdout, "--rdwr %s \n", rdwr_arg);
                fflush(stdout); 
            }
            break;
        case 'p'://pipe
            if (vb_flag) fprintf(stdout, "--pipe \n");
            fflush(stdout); 
            int pipefd[2];
            int p = pipe(pipefd);
            if (p == -1) {
                fprintf(stderr, "cannot create pipe\n");
                exit(1);
            }
            //printf("\n pipe1: %d, pipe2: %d", pipefd[0], pipefd[1]);
            fd_arr[fd_count+1] = pipefd[0];
            fd_arr[fd_count+2] = pipefd[1];
            fd_vld[fd_count+1] = 1;
            fd_vld[fd_count+2] = 1;
            fd_count += 2;            
            break;
    //subcommand
        case 'c'://command
            arg_begin = optind - 1;
            if (!isdigit(argv[arg_begin][0]) || !isdigit(argv[arg_begin+1][0]) || !isdigit(argv[arg_begin+2][0]) ){
                fprintf(stderr, "invalid file descriptor\n");
                exit(1);
            }
            i = atoi(argv[arg_begin]);
            o = atoi(argv[arg_begin+1]);
            e = atoi(argv[arg_begin+2]); 
            if (i<0||o<0||e<0||i>fd_count||o>fd_count||e>fd_count||!fd_vld[i]||!fd_vld[e]||!fd_vld[o]){
                fprintf(stderr, "invalid file descriptor\n");
                exit(1);
            }
            for ( j = 0; arg_begin+3+j < argc; j++){
                if (((argv[arg_begin+3+j][0] == '-') 
                && (argv[arg_begin+3+j][1] == '-'))
                ||(argv[arg_begin+3+j] == NULL)) break;
            } 

            for (k = 0; k < j; k++){
                options[k] = argv[arg_begin+3+k];
            }
            options[j] = NULL;

            if (options[0] == NULL) {
                fprintf(stderr,"there is no command\n");
                if (exit_stat == 0) exit_stat = 1;
            }
            //printf("\n command using fd: %d, %d, %d", fd_arr[i], fd_arr[o],fd_arr[e]);

            if (vb_flag){
                fprintf(stdout, "--command %d %d %d ", i, o, e);
                for (k = 0; k < j; k++) {
                    fprintf(stdout, "%s ", options[k]);
                }
                fprintf(stdout, "\n");
                fflush(stdout); 
            }

            optind = optind + 2 + j;
            
            char* optionsstr = NULL;
            int n = 0;
            for (l = 0 ; l < j; l++) {
                for (k = 0; options[l][k]; k++) {
                    n++;
                    optionsstr = realloc(optionsstr, n*sizeof(char));
                    optionsstr[n-1] = options[l][k];
                }
                n++;
                optionsstr = realloc(optionsstr, n*sizeof(char));
                optionsstr[n-1] = ' ';                
            }
            if (n != 0) optionsstr[n-1] = '\0';
            options_arr[prc_count] = optionsstr;

            pid_arr[prc_count] = fork();

            if (pid_arr[prc_count] < 0){
                fprintf(stderr, "cannot fork\n");
            }
            if (pid_arr[prc_count]==0){//pid = 0, child process
                close(0);
                close(1);
                close(2);
                dup2(fd_arr[i],0);
                dup2(fd_arr[o],1);
                dup2(fd_arr[e],2);
                close(fd_arr[i]);
                close(fd_arr[o]);
                close(fd_arr[e]);                
                for (k=0; k<fd_count+1; k++) {
                    if (fd_vld[k]){
                        close(fd_arr[k]);
                        fd_vld[k] = 0;
                    }
                }
                
                int xct = execvp(options[0],options);
                fflush(stdout); 
                if (xct<0) {
                    fprintf(stderr, "cannot execute the command\n"); 
                    exit_stat = 1;
                }
                prc_count++;
            }
            else if(pid_arr[prc_count]>0){//pid>0, parent process
                prc_count++;
            }


            break;
        case 'a'://wait
            if (vb_flag) {
                fprintf(stdout, "--wait \n");
                fflush(stdout); 
            }
            //printf("process count %d \n", prc_count);
            for (k = 0; k<prc_count; k++) {
                int pidstat;
                pid_t pidtermin;
                pidtermin = waitpid(-1, &pidstat, 0);
                //printf("terminating pid %d \n", pidtermin);
                if (WIFSIGNALED(pidstat)) {
                    for (l = 0; l < prc_count; l++){
                        if (pid_arr[l] == pidtermin) break;
                    }
                    fprintf(stdout, "signal %d ", WTERMSIG(pidstat));
                    fprintf(stdout, "%s ", options_arr[l]);
                    fprintf(stdout, "\n");
                    if (exit_stat < (WTERMSIG(pidstat) + 128)) exit_stat = WTERMSIG(pidstat) + 128;
                    //printf("exit status is %d now\n", exit_stat);
                    fflush(stdout);
                }
                else{
                    for (l = 0; l < prc_count; l++){
                        if (pid_arr[l] == pidtermin) break;
                    }
                    //printf("find process %d correspond to pidtermin", l);              
                    fprintf(stdout, "exit %d ", WEXITSTATUS(pidstat));
                    fprintf(stdout, "%s ", options_arr[l]);
                    fprintf(stdout, "\n");
                    exit_stat = (exit_stat > WEXITSTATUS(pidstat))? exit_stat : WEXITSTATUS(pidstat);
                    fflush(stdout);
                }
            }
            prc_count = 0;
            
            break;
    // miscellaneous    
        case 'l'://close
            cfd = atoi(optarg);
            if (cfd > fd_count || cfd < 0) {
                fprintf(stderr, "invalid file descriptor to close\n");
                exit(1);
            }
            fd_vld[cfd] = 0;
            close(fd_arr[cfd]);
            if (vb_flag) {
                fprintf(stdout, "--close %s \n",optarg);
                fflush(stdout);                
            }
            break;
        case 'b'://abort
            if (vb_flag) {
                fprintf(stdout, "--abort \n");
                fflush(stdout);
            }
            raise(SIGSEGV);
            break;
        case 'h'://catch
        {
            struct sigaction a1;
            a1.sa_handler = handler;
            catch_arg = atoi(optarg);
            if (vb_flag) {
                fprintf(stdout, "--catch %d \n", catch_arg);
                fflush(stdout);
            }
            sigaction(catch_arg, &a1, NULL);
            break;
        }
        case 'i'://ignore
        {
            struct sigaction a2;
            a2.sa_handler = SIG_IGN;
            ignore_arg = atoi(optarg);
            if (vb_flag) {
                fprintf(stdout, "--ignore %d \n", ignore_arg);
                fflush(stdout);
            }
            sigaction(ignore_arg, &a2, NULL);
            break;
        }
        case 'd'://default
        {
            struct sigaction a3;
            a3.sa_handler = SIG_DFL;
            default_arg = atoi(optarg);
            if (vb_flag) {
                fprintf(stdout, "--default %d \n", default_arg);
                fflush(stdout);
            }
            sigaction(default_arg, &a3, NULL);
            break;
        }
        case 'u'://pause
            if (vb_flag) {
                fprintf(stdout, "--pause \n");
                fflush(stdout);
            }
            pause();
            break;

        case 0: // all the argument
            break;
      
        default:
            fprintf(stderr, "invalid option\n");
            if (exit_stat < 1) exit_stat = 1;
        }
    }

    exit(exit_stat);
}
