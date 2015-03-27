/* Filename: main.c
 * Last Modified: 3/27/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */

#include "donuts.h"

/* Global Variables */
struct donut_ring shared_ring;
pthread_mutex_t prod [NUMFLAVORS];
pthread_mutex_t cons [NUMFLAVORS];
pthread_cond_t prod_cond [NUMFLAVORS];
pthread_cond_t cons_cond [NUMFLAVORS];
pthread_t thread_id [NUMCONSUMERS+NUMPRODUCERS], sig_wait_id;

/* main program */
int main(int argc, char *argv[]) {

    /* main variables */
    struct timeval randtime, first_time, last_time;
    int arg_array[NUMCONSUMERS+NUMPRODUCERS];

    /* variables for signal handling */
    struct sigaction new_act;
    sigset_t all_signals;
    int sigs[] = {SIGBUS, SIGSEGV, SIGFPE};
    int nsigs;

    /* structures for initialing threads: not mandatory */
    pthread_attr_t thread_attr;
    struct sched_param sched_struct;

    /* counters and temp variables */
    int i, j, k;

    /* Initial timestamp value for performance measurement */
    gettimeofday(&first_time, (struct timezone *) 0);

    /* array of argument values/thread id's*/
    for (i = 0; i < NUMCONSUMERS + NUMPRODUCERS; i++) {
        arg_array [i] = i + 1;
    }

    /* pthread and global value initialization */
    for (i = 0; i < NUMFLAVORS; i++) {
        pthread_mutex_init(&prod [i], NULL);
        pthread_mutex_init(&cons [i], NULL);
        pthread_cond_init(&prod_cond [i], NULL);
        pthread_cond_init(&cons_cond [i], NULL);
        shared_ring.out_ptr [i] = 0;
        shared_ring.in_ptr [i] = 0;
        shared_ring.serial [i] = 0;
        shared_ring.spaces [i] = NUMSLOTS;
        shared_ring.donuts [i] = 0;
    }

    printf("\nSetting up signal handling for main...\n");

    /**********************************************************************/
    /* SETUP FOR MANAGING THE SIGTERM SIGNAL, BLOCK ALL ASYNC SIGNALS     */
    /**********************************************************************/
    nsigs = sizeof ( sigs) / sizeof ( int);
    sigfillset(&all_signals);
    for (i = 0; i < nsigs; i++)
        sigdelset(&all_signals, sigs [i]);
    /* sigprocmask changes signal mask for calling thread (main) */
    sigprocmask(SIG_BLOCK, &all_signals, NULL);

    /* setup signal handlers for SIGBUS, SIGSEGV, SIGFPE */
    sigfillset(&all_signals);
    for (i = 0; i < nsigs; i++) {
        new_act.sa_handler = sig_handler;
        new_act.sa_mask = all_signals; /* block all signals in handler */
        new_act.sa_flags = 0;
        if (sigaction(sigs[i], &new_act, NULL) == -1) {
            perror("can't set signal handlers");
            exit(1);
        }
    }

    printf("Signal management complete\n");
    printf("Beginning thread creation...\n");

    /*********************************************************************/
    /* CREATE SIGNAL HANDLER THREAD                                      */
    /*********************************************************************/
    if (pthread_create(&sig_wait_id, NULL,
            sig_waiter, NULL) != 0) {
        printf("Failed to create signal waiter thread\n");
        exit(3);
    }
    printf("Signal waiter created...\n");

    /* set attributes for consumer and producer threads*/
    pthread_attr_init(&thread_attr);
    pthread_attr_setinheritsched(&thread_attr,
            PTHREAD_INHERIT_SCHED);

#ifdef  GLOBAL /* if GLOBAL, spread threads out on cores */
    sched_struct.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_attr_setinheritsched(&thread_attr,
            PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
    pthread_attr_setschedparam(&thread_attr, &sched_struct);
    pthread_attr_setscope(&thread_attr,
            PTHREAD_SCOPE_SYSTEM);
#endif

    /* start consumer and producer threads */
    for (i = 0; i < NUMCONSUMERS; i++) {
        if (pthread_create(&thread_id [i], &thread_attr,
                consumer, (void *) &arg_array [i]) != 0) {
            printf("Failed to created consumer %d\n", i);
            exit(3);
        } else 
           if (DEBUG) printf("Launching consumer thread %d\n",i+1);
    }
    printf("Consumer threads created...\n");

    for ( ; i < NUMCONSUMERS + NUMPRODUCERS; i++){	
        if (pthread_create(&thread_id[i], &thread_attr,
                producer, (void *) &arg_array [i]) != 0) {
            printf("Failed to create producer %d\n", i-NUMCONSUMERS);
            exit(3);
        } else 
           if ( DEBUG ) printf("Launching producer thread %d\n", arg_array[i]);
    }
    printf("Producer threads created...\nThread creation complete\n");

    /*********************************************************************/
    /* WAIT FOR ALL CONSUMERS TO FINISH, SIGNAL WAITER WILL              */
    /* NOT FINISH UNLESS A SIGTERM ARRIVES AND WILL THEN EXIT            */
    /* THE ENTIRE PROCESS....OTHERWISE MAIN THREAD WILL EXIT             */
    /* THE PROCESS WHEN ALL CONSUMERS ARE FINISHED                       */
    /*********************************************************************/
    printf("Waiting for consumers to finish...\n\n");
    for (i = 0; i < NUMCONSUMERS; i++) {
        pthread_join(thread_id [i], NULL);
        if (DEBUG) printf("Joined thread #%d", i);
    }

    /*****************************************************************/
    /* GET FINAL TIMESTAMP, CALCULATE ELAPSED SEC AND USEC           */
    /*****************************************************************/
    gettimeofday(&last_time, (struct timezone *) 0);
    if ((i = last_time.tv_sec - first_time.tv_sec) == 0)
        j = last_time.tv_usec - first_time.tv_usec;
    else {
        if (last_time.tv_usec - first_time.tv_usec < 0) {
            i--;
            j = 1000000 +
                    (last_time.tv_usec - first_time.tv_usec);
        } else {
            j = last_time.tv_usec - first_time.tv_usec;
        }
    }
    printf("Elapsed consumer time is %d sec and %d usec\n", i, j);

    printf("\n\n ALL CONSUMERS FINISHED, KILLING  PROCESS\n\n");
    exit(0);
} /* end main */

/**********************************************************/
/* PTHREAD SYNCH SIGNAL HANDLER ROUTINE                   */
/**********************************************************/
void sig_handler(int sig) {
    pthread_t signaled_thread_id;
    int i, thread_index;

    signaled_thread_id = pthread_self();
    for (i = 0; i < (NUMCONSUMERS + NUMPRODUCERS); i++) {
        if (signaled_thread_id == thread_id [i]) {
            thread_index = i;
            break;
        }
    }
    printf("\nThread %d took signal # %d, PROCESS HALT\n",
            thread_index, sig);
    exit(1);
} /* end sig_handler */


