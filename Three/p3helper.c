/* 
 * p3helper.c
 * Manuel Perez
 * Program 3
 * CS570
 * Dr. John Carroll
 * SDSU
 * 10/30/20
 *
 * This is the only file you may change. (In fact, the other files should
 * be symbolic links to:
 *   ~cs570/Three/p3main.c
 *   ~cs570/Three/p3.h
 *   ~cs570/Three/Makefile
 *   ~cs570/Three/CHK.h
 */

#include "p3.h"

sem_t jogger;	//semaphore for a jogger
sem_t shooter;	//semaphore for a shooter
sem_t enter_mutex;	//A semaphore for entering the gym
sem_t leave_mutex;	//A semaphore for exiting the gym

int res = 0;	//variable to store return value of the init command for a semaphore
int i;	//arbitrary variable to loop through a for loop
static int jogs = 0;	//Used to store the number of jogs entering/leaving the gym
static int shoots = 0;	//Used to store the number of shooters entering/leaving the gym

/* Used to initialize all of the semaphores */
void initstudentstuff(void) {

    /* initializing jogger semaphore */
    if ((res = sem_init(&jogger, LOCAL, 1)) < 0) {
	fprintf(stderr, "failed to initialize jogger semaphore.n");
    }

    /* initializing shooter semaphore */
    if ((res = sem_init(&shooter, LOCAL, 1)) < 0) {
        fprintf(stderr, "failed to initialize shooter semaphore.n");
    }

    /* initializing enter_mutex semaphore */
    if ((res = sem_init(&enter_mutex, LOCAL, 1)) < 0) {
        fprintf(stderr, "failed to initialize enter_mutex semaphore.n");
    }
    
    /* initializing leave_mutex semaphore */
    if ((res = sem_init(&leave_mutex, LOCAL, 1)) < 0) {
	fprintf(stderr, "failed to initialize leave_mutex semaphore.n");
    }
}

/* Manages who can enter the gym depending on who is currently in the gym */
void prolog(int kind) {

    if (kind == JOGGER) {	//Checking to see if you're a jogger entering gym
	CHK(sem_wait(&enter_mutex));	//entering critical region for entering gym

	jogs++;	//A jogger is trying to enter the gym

	/* check if there's already shooters in the gym, if so block the jogger */
	if (shoots > 0) CHK(sem_wait(&jogger));

	/* 
	 * If you're the first jogger and there's no shooters in the gym
	 * decrease their semaphore by 1, so the next shooter coming in will
	 * get blocked
	 */
	if (jogs == 1 && shoots == 0) CHK(sem_wait(&shooter));

	CHK(sem_post(&enter_mutex));	//exiting critical region for entering gym
    }

    else if (kind == SHOOTER) {	//Checking to see if you're a shooter entering gym
	CHK(sem_wait(&enter_mutex));	//entering critical region for entering gym

	shoots++;	//A shooter is trying to enter the gym

	/* check if there's already shooters in the gym, if so block the shooter */
	if (jogs > 0) CHK(sem_wait(&shooter));

	/*
         * If you're the first shooter and there's no joggers in the gym
         * decrease their semaphore by 1, so the next jogger coming in will
         * get blocked
         */
	if (shoots == 1 && jogs == 0) CHK(sem_wait(&jogger));

	CHK(sem_post(&enter_mutex));	//exiting critical region for entering gym
    }

}

/* 
 * Manages people leaving the gym and allows access to a group of people
 * if they were blocked from entering 
 */
void epilog(int kind) {

    if (kind == JOGGER) {	//Checking to see if you're a jogger leaving gym
        CHK(sem_wait(&leave_mutex));	//entering critical region for leaving gym

	jogs--;	//A jogger is trying to leave the gym
	if (jogs == 0) {	//Checking to see if you're the last jogger leaving gym
	    
	    /* 
	     * loop through all of the shooters that were trying to enter the gym
	     * and unblock them. If there were no shooters waiting, this does nothing
	     */ 
	    for (i = 0; i <= shoots; i++) {
		CHK(sem_post(&shooter)); //Unblocking shooters who were trying to enter gym
	    }
	}
        CHK(sem_post(&leave_mutex));	//exiting critical region for leaving gym
    }

    else if (kind == SHOOTER) {	//Checking to see if you're a shooter leaving
        CHK(sem_wait(&leave_mutex));	//entering critical region for leaving gym

	shoots--;	//A shooter is trying to leave the gym
	if (shoots == 0) {	//Checking to see if you're the last jogger leaving gym

	    /*
             * loop through all of the joggers that were trying to enter the gym
             * and unblock them. If there were no joggers waiting, this does nothing
             */
	    for (i = 0; i <= jogs; i++) {
		CHK(sem_post(&jogger));
	    }	
	}

        CHK(sem_post(&leave_mutex));	//exiting critical region for leaving gym
    }

}
