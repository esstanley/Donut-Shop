/* Filename: donuts.h
 * Last Modified: 3/21/2015
 * 
 * Assignment 4 
 * Eugene Stanley
 * eugene.stanley@gmail.com
 */

#ifndef DONUTS_H
#define	DONUTS_H

#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define         NUMFLAVORS       	4
#define         NUMSLOTS        	600
#define         NUMCONSUMERS     	50
#define		NUMPRODUCERS		30
#define         DOZENS                  200
#define         DEBUG                   0       /* 1=ON, 0=OFF */

/* Ring buffer */
struct donut_ring {
    int flavor [NUMFLAVORS] [NUMSLOTS];
    int out_ptr [NUMFLAVORS];
    int in_ptr [NUMFLAVORS];
    int serial [NUMFLAVORS];
    int spaces [NUMFLAVORS];
    int donuts [NUMFLAVORS];
};

/* Thread functions and sig_handler function */
void *sig_waiter(void *arg);
void *producer(void *arg);
void *consumer(void *arg);
void sig_handler(int);

#endif	/* DONUTS_H */

