#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include "SortedList.h"

int opt_yield;

static int niteration =1;
static SortedListElement_t *op_arr;
static pthread_mutex_t lock;
static int spinlock;
static char opt_sync = 'p';
static SortedList_t list; // a list used for insert


char* generatekey(){
  char *k = malloc(7*sizeof(char)); 
  for (int j = 0; j < 6; j++)
    k[j] = (char) rand() % 90 + '0';
  k[6] = '\0';
  return k;
}

void *thread_routine(void* th){
  int i;
  int thread_begin = *(int *)th;
  int thread_end = thread_begin + niteration;
  //insert
  for (i = thread_begin; i < thread_end; i++){
    if (opt_sync == 'm') pthread_mutex_lock(&lock);
    else if (opt_sync == 's') {
      while (__sync_lock_test_and_set(&spinlock, 1)){;}
    }
    SortedList_insert(&list, &op_arr[i]);
    if (opt_sync == 'm') pthread_mutex_unlock(&lock);
    else if (opt_sync == 's') __sync_lock_release(&spinlock);
  }

  //list_length
  if (opt_sync == 'm') pthread_mutex_lock(&lock);
  else if (opt_sync == 's') {
    while (__sync_lock_test_and_set(&spinlock, 1)){;}
  }
  int list_length = SortedList_length(&list);
  if (list_length < 0) exit(2);
  if (opt_sync == 'm') pthread_mutex_unlock(&lock);
  else if (opt_sync == 's') __sync_lock_release(&spinlock);
    
  //lookup & delete
  for(i = thread_begin; i < thread_end; i++){
    if (opt_sync == 'm') pthread_mutex_lock(&lock);
    else if (opt_sync == 's') {
      while (__sync_lock_test_and_set(&spinlock, 1)){;}
    }
    SortedListElement_t *element = SortedList_lookup(&list, op_arr[i].key);
    if (!element || SortedList_delete(element) != 0) exit(2);
    if (opt_sync == 'm') pthread_mutex_unlock(&lock);
    else if (opt_sync == 's') __sync_lock_release(&spinlock);
  }
  return NULL;
}

int main(int argc, char **argv){
  struct option long_options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", required_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0,0,0,0}
  };
    
  int nthread = 1;
  int i;
  int opt_index;
  struct timespec starttime;
  struct timespec endtime;
  opt_yield = 0;

  while(1){
    int c = getopt_long(argc, argv, "", long_options, &opt_index);
    if (c == -1) break;
    switch (c)
      {
      case 't':
	nthread = atoi(optarg);
	break;
      case 'i':
	niteration = atoi(optarg);
	break;
      case 'y':
	for (i = 0; optarg[i] != '\0'; i++){
	  if (optarg[i] == 'i') opt_yield |= INSERT_YIELD;
	  else if (optarg[i] == 'd') opt_yield |= DELETE_YIELD;
	  else if (optarg[i] == 'l') opt_yield |= LOOKUP_YIELD;
	  else {
	    fprintf(stderr, "invalid yield option");
	    exit(1);
	  }
	}
	break;
      case 's':
	if (optarg[0] == 'm' || optarg[0] == 's'){
	  opt_sync = optarg[0];
	}
	else {
	  fprintf(stderr, "invalid option for sync");
	  exit(1);
	}
	break;    
        
      default:
	fprintf(stderr, "invalid option\n");
	exit(1);
      }

  }

  SortedList_t *head = &list;
  list.prev = head;
  list.next = head;
  list.key = NULL;

  int noperation = nthread * niteration;
  op_arr = malloc(noperation * sizeof(SortedListElement_t));
  srand(time(NULL));
  for (i = 0; i < noperation; i++) op_arr[i].key = generatekey();

  int *thread_head = malloc(nthread * sizeof(int));
  for (i = 0; i < nthread; i++) thread_head[i] = i * niteration;

  if (clock_gettime(CLOCK_MONOTONIC, &starttime)){
    fprintf(stderr, "clock_gettime error");
    exit(1);
  }
  pthread_t* t_arr = (pthread_t*)malloc(sizeof(pthread_t)*nthread);
  if (!t_arr){
    fprintf(stderr, "memory allocation error");
    free(thread_head);
    free(op_arr);
    exit(1);
  }
  for (i = 0; i < nthread; i++){
    if (pthread_create(&t_arr[i], NULL, thread_routine, &thread_head[i])){
      fprintf(stderr, "thread create error");
      free(thread_head);
      free(op_arr);
      free(t_arr);
      exit(1);
    }
  }
  for (i = 0; i < nthread; i++ ){
    if (pthread_join(t_arr[i], NULL)){
      fprintf(stderr, "thread join error");
      free(thread_head);
      free(op_arr);
      free(t_arr);
      exit(1);
    }
  }
  if (clock_gettime(CLOCK_MONOTONIC, &endtime)){
    fprintf(stderr, "clock_gettime error");
    free(thread_head);
    free(op_arr);
    free(t_arr);
    exit(1);
  }
  long long runtime = endtime.tv_nsec - starttime.tv_nsec + (endtime.tv_sec - starttime.tv_sec) * 1000000000;

  if (SortedList_length(&list) != 0){
    free(thread_head);
    free(op_arr);
    free(t_arr);
    exit(2);
  }

  fprintf(stdout, "list-");
  if (opt_yield & INSERT_YIELD) fprintf(stdout, "i");
  if (opt_yield & DELETE_YIELD) fprintf(stdout, "d");
  if (opt_yield & LOOKUP_YIELD) fprintf(stdout, "l");
  if (opt_yield == 0) fprintf(stdout, "none");
    
  if (opt_sync == 'p') fprintf(stdout, "-none,");
  else fprintf(stdout, "-%c,", opt_sync);

  noperation = noperation * 3;
  long long avgtime = runtime/noperation;
  fprintf(stdout, "%d,%d,1,%d,%lld,%lld\n",
	  nthread, niteration, noperation, runtime, avgtime);

  free(thread_head);
  free(op_arr);
  free(t_arr);
  exit(0);    
}
