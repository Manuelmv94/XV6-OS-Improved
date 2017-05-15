#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX 10
int buffer[MAX];
sem_t sem_mutex;
sem_t sem_ready;
sem_t sem_slots;

void *producer(void *threadid){
  int i=0;
  while(1){

    sem_wait(&sem_slots);
    sem_wait(&sem_mutex);
    buffer[i%MAX]=i++;
    sem_post(&sem_ready);
    sem_post(&sem_mutex);

  }
}

void *consumer(void *threadid){
  int i=0;
  int value;
  while(1){
    sem_wait(&sem_ready);
    sem_wait(&sem_mutex);
    value=buffer[(i++)%MAX];
    sem_post(&sem_slots);
    sem_post(&sem_mutex);

    printf("value is %d\n",value);
  }
}


int main(){

  pthread_t pt_consumer;
  pthread_t pt_producer;

  sem_init(&sem_mutex,0,1);
  sem_init(&sem_ready,0,0);
  sem_init(&sem_slots,0,MAX);

  pthread_create(&pt_producer,NULL,producer,NULL);
  pthread_create(&pt_consumer,NULL,consumer,NULL);

  pthread_exit(NULL);
  return 0;
}
