#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sched.h>

static pthread_mutex_t lock;
static int spinlock;
static int opt_yield;

static long long *counter;
static int niteration;
static char opt_sync = 'p';

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

void* thread_routine(){
  int i;
  for (i = 0; i < niteration; i++){
    if (opt_sync == 'p') add(counter, 1);
    else if (opt_sync == 'm'){
      pthread_mutex_lock(&lock);
      add(counter, 1);
      pthread_mutex_unlock(&lock);
    }
    else if (opt_sync == 's'){
      while (__sync_lock_test_and_set(&spinlock, 1)){;}
      add(counter, 1);
      __sync_lock_release(&spinlock);
    }
    else if (opt_sync == 'c'){
      while(1){
	int now = *counter;
	int next = now + 1;
	if (__sync_val_compare_and_swap(counter, now, next) == now) break;
      }
    }        
  }
  //add 1 to the counter the specified number of times
  for (i = 0; i < niteration; i++){
    if (opt_sync == 'p') add(counter, -1);
    else if (opt_sync == 'm'){
      pthread_mutex_lock(&lock);
      add(counter, -1);
      pthread_mutex_unlock(&lock);
    }
    else if (opt_sync == 's'){
      while (__sync_lock_test_and_set(&spinlock, -1)){;}
      add(counter, -1);
      __sync_lock_release(&spinlock);
    }
    else if (opt_sync == 'c'){
      while(1){
	int now = *counter;
	int next = now - 1;
	if (__sync_val_compare_and_swap(counter, now, next) == now) break;
      }
    }        
  }
  //add -1 to the counter the specified number of times
  return NULL;
}

int main(int argc, char **argv){
  struct option long_options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", no_argument, &opt_yield, 1},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };
  int nthread = 1;
  int opt_index;
  struct timespec starttime;
  struct timespec endtime;
  long long cnt;

  while(1){
    int c = getopt_long(argc, argv, "", long_options, &opt_index);
    if (c == -1) break;
    switch(c){
    case 't':
      nthread = atoi(optarg);
      break;
    case 'i':
      niteration = atoi(optarg);
      break;
    case 's':
      if (optarg[0] == 'm' || optarg[0] == 's' || optarg[0] == 'c'){
	opt_sync = optarg[0];
      }
      else {
	fprintf(stderr, "invalid option for sync");
	exit(1);
      }
      break;
    case 0:
      break;
    default:
      fprintf(stderr, "invalid option\n");
      exit(1);
    }
  }
  counter = &cnt;

  if (clock_gettime(CLOCK_MONOTONIC, &starttime)){
    fprintf(stderr, "clock_gettime error");
    exit(1);
  }
  pthread_t* t_arr = (pthread_t*)malloc(sizeof(pthread_t)*nthread);
  if (!t_arr){
    fprintf(stderr, "memory allocation error");
    exit(1);
  }
  int i;
  for (i = 0; i < nthread; i++){
    if (pthread_create(&t_arr[i], NULL, thread_routine, NULL)){
      fprintf(stderr, "thread create error");
      exit(1);
    }
  }
  for (i = 0; i < nthread; i++ ){
    if (pthread_join(t_arr[i], NULL)){
      fprintf(stderr, "thread join error");
      exit(1);
    }
  }
  if (clock_gettime(CLOCK_MONOTONIC, &endtime)){
    fprintf(stderr, "clock_gettime error");
    exit(1);
  }
  long long runtime = endtime.tv_nsec - starttime.tv_nsec + (endtime.tv_sec - starttime.tv_sec) * 1000000000;
  //print out CSV11
  //the name of the test (add-none for the most basic usage)
  if (opt_yield && (opt_sync == 'p')){
    fprintf(stdout, "add-yield-none,");
  }
  else if (opt_yield){
    fprintf(stdout, "add-yield-%c,", opt_sync);
  }
  else if (opt_sync == 'p'){
    fprintf(stdout, "add-none,");
  }
  else {
    fprintf(stdout, "add-%c,", opt_sync);
  }
  long long noperation = nthread * niteration * 2;
  long long avgtime = runtime/noperation;
  fprintf(stdout, "%d,%d,%lld,%lld,%lld,%lld\n",
	  nthread, niteration, noperation, runtime, avgtime, cnt);
  /*    
    the number of threads (from --threads=)
    the number of iterations (from --iterations=)
    the total number of operations performed (threads x iterations x 2, the "x 2" factor because you add 1 first and then add -1)
    the total run time (in nanoseconds)
    the average time per operation (in nanoseconds).
    the total at the end of the run (0 if there were no conflicting updates)
  */
  free(t_arr);
  if (cnt != 0) exit(2);
  exit(0);
}
