#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

int randomInt (int max){
  return (int)( ( (1.0*rand())/RAND_MAX ) * max + 1);
}
#define NUM_PHIL 100
sem_t forks[NUM_PHIL];
sem_t chairs;

void *phil(void *threadid){
  long id=(long)threadid;
  unsigned time_thinking=1;
  unsigned time_eating=1;
  while(1){
    printf("phil %ld thinking\n",id);
    time_thinking=randomInt(5);
    sleep(time_thinking);

    sem_wait(&chairs);
    sem_wait(&forks[id]);
    sem_wait(&forks[(id+1)%NUM_PHIL]);

    printf("phil %ld eating\n",id);

    time_eating=randomInt(5);
    sleep(time_eating);
    sem_post(&forks[id]);
    sem_post(&forks[(id+1)%NUM_PHIL]);
    sem_post(&chairs);
  }
}


int main(){
  long t;
  int rc;
  pthread_t phils[NUM_PHIL];
  srand(time(NULL));
  sem_init(&chairs,0,NUM_PHIL-1);

  for(t=0;t<NUM_PHIL;t++){
    sem_init(&forks[t],0,1);
  }

  for(t=0;t<NUM_PHIL;t++){

    rc = pthread_create(&phils[t], NULL, phil, (void *)t);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  pthread_exit(NULL);
  return 0;
}
