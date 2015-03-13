/* Filename: sigwaiter.c
 * Last Modified: 3/21/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */


#include "donuts.h"

/***********************************************************/
/* PTHREAD ASYNCH SIGNAL HANDLER ROUTINE...                */
/***********************************************************/

void *sig_waiter(void *arg) {
    sigset_t sigterm_signal;
    int signo;

    sigemptyset(&sigterm_signal);
    sigaddset(&sigterm_signal, SIGTERM);
    sigaddset(&sigterm_signal, SIGINT);

    if (sigwait(&sigterm_signal, & signo) != 0) {
            printf("\n  sigwait ( ) failed, exiting \n");
            exit(2);
        }
    printf("process going down on SIGNAL (number %d)\n\n", signo);
    exit(1);
    return NULL;
}
