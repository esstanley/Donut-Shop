/* Filename: producer.c
 * Last Modified: 3/27/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */


#include "donuts.h"

/* global variables defined in main */ 
extern struct donut_ring shared_ring;
extern pthread_mutex_t prod [NUMFLAVORS];
extern pthread_mutex_t cons [NUMFLAVORS];
extern pthread_cond_t prod_cond [NUMFLAVORS];
extern pthread_cond_t cons_cond [NUMFLAVORS];
extern pthread_t thread_id [NUMCONSUMERS + 1], sig_wait_id;

/*********************************************/
/*                PRODUCER                   */
/*********************************************/
void *producer(void *arg) {

    /* main variables */

    /* id number of current producer */
    int id;
    
    /* variables for generating a random seed */
    unsigned short xsub [3];
    struct timeval randtime;

    /* counter and temp variables */
    int i, j, k;
    
    /* get producer id and display entry information */
    id = *(int *) arg;
    if ( DEBUG ) {
        printf("Entering producer %d\n", id );
    }
    
    /* generate random seed using microsecond for uniquness */
    gettimeofday(&randtime, (struct timezone *) 0);
    xsub [0] = (ushort) randtime.tv_usec;
    xsub [1] = (ushort) (randtime.tv_usec >> 16);
    xsub [2] = (ushort) (pthread_self());
    
    /* donut generating loop */
    while (1) {

        /* choose donut type */
        j = nrand48(xsub) & 3;
        
        /* grab producer mutex, sleep if no space to put donut */
        pthread_mutex_lock(&prod [j]);
        while (shared_ring.spaces [j] == 0) {
            pthread_cond_wait(&prod_cond [j], &prod [j]);
        }
        
        /* put the donut in the box, adjust in_ptr, serial, and spaces */
        shared_ring.flavor[j][shared_ring.in_ptr[j]] = shared_ring.serial[j];
        shared_ring.serial[j]++;
        shared_ring.in_ptr[j] = ( shared_ring.in_ptr[j] + 1 ) % NUMSLOTS;
        shared_ring.spaces[j]--;
        pthread_mutex_unlock(&prod [j]);
        if (DEBUG) {
            printf("PRODUCER#%d: Donut #%d of type %d placed in ring buffer %d\n",
                    id, shared_ring.serial[j], j, j);
            fflush(stdout);
        }
        
        /* get consumer mutex, increment donuts[j], signal waiting consumers */
        pthread_mutex_lock(&cons[j]);
        shared_ring.donuts[j]++;
        pthread_mutex_unlock(&cons[j]);
        pthread_cond_signal(&cons_cond[j]);
        if (DEBUG) {
            printf("PRODUCER#%d: Donuts[%d] incremented, consumers signalled\n",
                    id, j);
            fflush(stdout);
        }
    } /* end donut generating loop */

    /* will never get here */
    return NULL;

} /* end producer thread function */

