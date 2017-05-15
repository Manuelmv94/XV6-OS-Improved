#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NUM_THREADS	5

pthread_mutex_t saldo_mutex;
unsigned int saldo;

void *PrintHello(void *threadid)
{
   long tid;
   unsigned misaldo;
   tid = (long)threadid;
   //printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_mutex_lock(&saldo_mutex);
   misaldo=saldo;
   misaldo+=1;
   if(tid%2==0)sleep(1);
   saldo=misaldo;
   pthread_mutex_unlock(&saldo_mutex);

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   int num_threads=atoi(argv[1]);
   pthread_t threads[num_threads];
   int rc;
   long t;
   pthread_mutex_init(&saldo_mutex,NULL);
   for(t=0;t<num_threads;t++){
     //printf("In main: creating thread %ld\n", t);
     rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
     if (rc){
       printf("ERROR; return code from pthread_create() is %d\n", rc);
       exit(-1);
       }
   }
   for(t=0;t<num_threads;t++){
     pthread_join(threads[t], NULL);
   }

   printf("saldo %u \n",saldo);
   /* Last thing that main() should do */
   pthread_exit(NULL);
}
