/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define TEST_STRING2 "Dont print!"
#define OFFSET2 1000
#define OFFSET_bad_1 500
#define OFFSET_bad_2 1500


/* proc1 writes some data, commits it, then exits */
void proc1()
{
     rvm_t rvm;
     trans_t trans,trans2;
     char* segs[1];

     rvm = rvm_init("rvm_segments");
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);


     trans = rvm_begin_trans(rvm, 1, (void **) segs);

     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0], TEST_STRING);

     rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
     sprintf(segs[0]+OFFSET2, TEST_STRING);

     rvm_commit_trans(trans);

     //2nd transaction

     segs[0] =(char *) rvm_map(rvm, "testseg", 10000);

     trans2 = rvm_begin_trans(rvm, 1, (void **) segs);

     rvm_about_to_modify(trans2, segs[0], OFFSET_bad_1, 100);
     sprintf(segs[0], TEST_STRING);

     rvm_about_to_modify(trans2, segs[0], OFFSET_bad_2, 100);
     sprintf(segs[0]+OFFSET2, TEST_STRING);

     rvm_abort_trans(trans2);

     abort();
}


/* proc2 opens the segments and reads from them */
void proc2()
{
     char* segs[1];
     rvm_t rvm;

     rvm = rvm_init("rvm_segments");

     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     if(strcmp(segs[0], TEST_STRING)) {
	  printf("ERROR: first hello not present\n");
	  exit(2);
     }
     if(strcmp(segs[0]+OFFSET2, TEST_STRING)) {
	  printf("ERROR: second hello not present\n");
	  exit(2);
     }

     if(!strcmp(segs[0]+OFFSET_bad_1, TEST_STRING)) {
      printf("ERROR: 1st bad offset present\n");
      exit(2);
     }

     if(!strcmp(segs[0]+OFFSET_bad_2, TEST_STRING)) {
      printf("ERROR: 2nd bad offset present\n");
      exit(2);
     }

     printf("OK\n");
     exit(0);
}


int main(int argc, char **argv)
{
     int pid;

     pid = fork();
     if(pid < 0) {
	  perror("fork");
	  exit(2);
     }
     if(pid == 0) {
	  proc1();
	  exit(0);
     }

     waitpid(pid, NULL, 0);

     proc2();

     return 0;
}
