#include "types.h"
#include "user.h"

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
#define FEA (argv[1][0]=='-' && argv[1][1]=='f' && argv[1][2]=='e' && argv[1][3]=='a')
struct proc proc[NPROC];

static char *states[] = {
[UNUSED]    "unused",
[EMBRYO]    "embryo",
[SLEEPING]  "sleep ",
[RUNNABLE]  "runble",
[RUNNING]   "run   ",
[ZOMBIE]    "zombie"
};


struct proc *p;
char *state;
uint time;

int main(int argc, char *argv[]){


  list_procs(proc,sizeof(proc));
  if(FEA){
    printf(1,"%s  ", "PPID");
  }

  printf(1,"%s   %s   %s", "PID", "STATE", "CMD");

  if(FEA){
    printf(1,"     %s", "STIME");
  }

  printf(1,"\n");

  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";

    if(FEA){
      printf(1,"%d    ", p->ppid);
    }

    printf(1,"%d     %s  %s ", p->pid, state, p->name);

    if(FEA){          //Conversion to time taking quantum as 20uS
      time=(p->times);
      printf(1,"     %d.", time/50000);
      time=time%50000;
      time/=50;
      if (time<100){
        printf(1,"0");
      }
      if (time<10){
        printf(1,"0");
      }
      printf(1,"%d", time);
    }

    printf(1,"\n");


  }
	exit();
}
