/* Filename: consumer.c
 * Last Modified: 3/21/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */


#include "donuts.h"

extern struct donut_ring shared_ring;
extern pthread_mutex_t prod [NUMFLAVORS];
extern pthread_mutex_t cons [NUMFLAVORS];
extern pthread_cond_t prod_cond [NUMFLAVORS];
extern pthread_cond_t cons_cond [NUMFLAVORS];
extern pthread_t thread_id [NUMCONSUMERS + 1], sig_wait_id;

void output_dozen(int dozen_num, int dozen[2][12]);

void *consumer(void *arg) {
    int i, j, k, m, id;
    unsigned short xsub [3];
    struct timeval randtime;
    id = *(int *) arg;
    printf("Entering consumer %d\n", id );
    /* dozen[][]: stores a dozen donuts
     * dozen[0] is the donut type
     * dozen[1] is the donut id # */
    int dozen[2][12];

    gettimeofday(&randtime, (struct timezone *) 0);
    xsub [0] = (ushort) randtime.tv_usec;
    xsub [1] = (ushort) (randtime.tv_usec >> 16);
    xsub [2] = (ushort) (pthread_self());

    for (i = 0; i < DOZENS; i++) {
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
        output_dozen(i, dozen);
        usleep(1000); /* sleep 1 ms */
    }
    
    return NULL ;
} /* end consumer thread function */

/* output_dozen() : displays thread id and types of donuts selected
                    for a particular dozen
 dozen_num: number of the dozen being displayed
 dozen[][]: contains types and id number of donuts selected
 */
void output_dozen(int dozen_num, int dozen[2][12]) {
    
    /* variables for getting and displaying current time */
    time_t cur_time;
    struct tm *form_time;
    int ms;
    char timebuffer[9];
    char outbuffer[20];
    /* counter variables */
    int i, j;

    time(&cur_time);
    ms = (cur_time % 1000000) / 1000;
    form_time = localtime(&cur_time);


    strftime(timebuffer, sizeof (timebuffer),
            "%H:%M:%S", form_time);
    sprintf(outbuffer, "%s:%d", timebuffer, ms);
    printf("Consumer thread ID: %d\ttime: %s\tdozen#: %d\n",
            pthread_self(), outbuffer, dozen_num);

    for (i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                printf("plain:     ");
                break;
            case 1:
                printf("jelly:     ");
                break;
            case 2:
                printf("chocolate: ");
                break;
            case 3:
                printf("honey-dip: ");
                break;
        }
        for (j = 0; j < 12; j++) {
            if (i == dozen[0][j])
                printf("%d\t", dozen[1][j]);
        }
        printf("\n");
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
    /*close(fp);*/

}