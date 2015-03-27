/* Filename: consumer.c
 * Last Modified: 3/27/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */


#include "donuts.h"
#include <sys/file.h>

/* Global Variables defined in main */
extern struct donut_ring shared_ring;
extern pthread_mutex_t prod [NUMFLAVORS];
extern pthread_mutex_t cons [NUMFLAVORS];
extern pthread_cond_t prod_cond [NUMFLAVORS];
extern pthread_cond_t cons_cond [NUMFLAVORS];
extern pthread_t thread_id [NUMCONSUMERS + 1], sig_wait_id;

/* function header for displaying a dozen */
void output_dozen(int cons_num, int dozen_num, int dozen[2][12], FILE *outfp);

/* main consumer function */
void *consumer(void *arg) {

    /***** main variables *****/

    /* holds id# of current thread */
    int id;

    /* variables used in random number generation */
    struct timeval randtime;
    unsigned short xsub [3];

    /* dozen[][]: stores a dozen donuts
     * dozen[0] is the donut type
     * dozen[1] is the donut id # */
    int dozen[2][12];

    /* variables for file manipulation */
    FILE *outfp;
    char fname[15];

    /* counter and temp variables */
    int i, j, k, m;
    
    /* display consumer startup information */
    id = *(int *) arg;
    if ( DEBUG ) {
        printf("Entering consumer %d\n", id );
    }

    /* get file ready for output */
    sprintf(fname, "consumer%d.txt", id);
    if ((outfp = fopen(fname, "w")) == NULL) {
        fprintf(stderr, "Can't open file to write consumer%d data!\n", id);
    }

    /* generate seed to be used in random number generator.
       use microsecond component for uniqueness */
    gettimeofday(&randtime, (struct timezone *) 0);
    xsub [0] = (ushort) randtime.tv_usec;
    xsub [1] = (ushort) (randtime.tv_usec >> 16);
    xsub [2] = (ushort) (pthread_self());

    for (i = 0; i < DOZENS; i++) {
	/* pick 12 donuts */
        for (m = 0; m < 12; m++) {

            /* j = random donut type */
            j = nrand48(xsub) & 3;

            /* grab consumer mutex, sleep if no donuts */
            pthread_mutex_lock(&cons[j]);
            while (shared_ring.donuts[j] == 0) {
                pthread_cond_wait(&cons_cond[j], &cons[j]);
            }

            /* take a donut from the box, adjust out_ptr, and donuts */
            dozen[0][m] = j;
            dozen[1][m] = shared_ring.flavor[j][shared_ring.out_ptr[j]];
            shared_ring.out_ptr[j] = ( shared_ring.out_ptr[j] + 1 ) % NUMSLOTS;
            shared_ring.donuts[j]--;
            pthread_mutex_unlock(&cons [j]);

            /* get producer mutex, increment space[j], signal waiting producers */
            pthread_mutex_lock(&prod[j]);
            shared_ring.spaces[j]++;
            pthread_mutex_unlock(&prod[j]);
            pthread_cond_signal(&prod_cond[j]);
        }

        /* output the previous dozen and force a context switch*/
        output_dozen(id, i, dozen, outfp);
        usleep(100000); /* sleep 1 ms */
    }
   
    /* close the output file */
    if ( outfp != NULL )
        close(outfp);
 
    return NULL ;
} /* end consumer thread function */

/* output_dozen() : displays thread id and types of donuts selected
                    for a particular dozen
 dozen_num: number of the dozen being displayed
 dozen[][]: contains types and id number of donuts selected
 */
void output_dozen(int cons_num, int dozen_num, int dozen[2][12], FILE *outfp) {
    
    /* variables for getting and displaying current time */
    struct timeval cur_time;
    char outbuffer[20];
    struct tm *ltime;

    /* counter variables */
    int i, j;

    /* return immediately if no file to write to */
    if (outfp==NULL)
        return;

    /* generate time stamp */
    gettimeofday(&cur_time, NULL);
    ltime = localtime(&cur_time.tv_sec);
    sprintf(outbuffer, "%d:%02d:%02d:%d", ltime->tm_hour, ltime->tm_min,
                          ltime->tm_sec, cur_time.tv_usec);

    /* write dozen to file */
    fprintf(outfp, "Consumer thread number: %d\tTime: %s\nDozen#: %d\n",
            cons_num, outbuffer, dozen_num+1);
    for (i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                fprintf(outfp, "plain:     ");
                break;
            case 1:
                fprintf(outfp, "jelly:     ");
                break;
            case 2:
                fprintf(outfp, "chocolate: ");
                break;
            case 3:
                fprintf(outfp, "honey-dip: ");
                break;
        }
        for (j = 0; j < 12; j++) {
            if (i == dozen[0][j])
                fprintf(outfp, "\t%d", dozen[1][j]);
        }
        fprintf(outfp, "\n");
    }
    fprintf(outfp, "\n");

} /* end output_dozen() */

