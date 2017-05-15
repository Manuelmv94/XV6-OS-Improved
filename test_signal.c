#include "types.h"

#include "user.h"

int value = 1;

void sig_handled(void){

    printf(1,"Signal received \n");

    value = 1;

    printf(1,"value = %d",value);

}

void sig_handled2(void){

    printf(1,"Signal received \n");

    value = 0;

    printf(1,"value = %d",value);

}

int main(int argc, char *argv[]){

    int pid;

    int signum;



    if(argc <= 1){

           printf(2,"Error use test_signal signum \n");

       exit();

     }

     signum = atoi(argv[1]);



    //Creating another process

    pid = fork();

    switch(pid){

        case -1: //There was a problem

            printf(2,"Error running fork");

            break;

        case 0: //child

            set_priority(-1);
            printf(1,"I am the child\n");

            printf(1,"Waiting for a signal \n");
            printf(2,"%d \n",sig_handled);
            //Setup the signal and wait for it

            set_signal(1,sig_handled2);

            while(value){};

            printf(1,"\nWhile completed\n");

            break;

        default://Parent

            //Wait for the child to start

            sleep(10);

            //Send the signal

            printf(1,"Sending signal %d to %d\n",signum,pid);

            kill_signal(signum,pid);

            //Wait for the child to complete

            wait();

            printf(1,"Child completed\n");

    }

    exit();

}
