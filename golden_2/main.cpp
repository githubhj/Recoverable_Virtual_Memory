/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1()
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
     
     rvm = rvm_init("rvm_segments");
     cout<<"P1 init done"<<endl;
     rvm_destroy(rvm, "testseg");
     cout<<"P1 destroy done"<<endl;
     cout<<"Calling map on P1"<<endl;

     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     cout<<"None of the stuff below executes before proc2()"<<endl;
     cout<<"WTF!!!!!"<<endl;
     print_segment_andaddr_map();
     cout<<"P1 map done"<<endl;
     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     cout<<"P1 begin trans done"<<endl;

     rvm_about_to_modify(trans, segs[0], 0, 100);
     cout<<"P1 atm 1 done"<<endl;
     sprintf(segs[0], TEST_STRING);

     rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
     cout<<"P1 atm 2 done"<<endl;
     sprintf(segs[0]+OFFSET2, TEST_STRING);

     rvm_commit_trans(trans);
     cout<<"P1 commit done"<<endl;

     abort();
}


/* proc2 opens the segments and reads from them */
void proc2()
{
     char* segs[1];
     rvm_t rvm;

     rvm = rvm_init("rvm_segments");
     cout<<"P2 init done"<<endl;


     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     cout<<"P2 map done"<<endl;
     cout<<"Data 1 is "<<segs[0];
     if(strcmp(segs[0], TEST_STRING)) {
	  printf("ERROR: first hello not present\n");
	  exit(2);
     }
     if(strcmp(segs[0]+OFFSET2, TEST_STRING)) {
	  printf("ERROR: second hello not present\n");
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
