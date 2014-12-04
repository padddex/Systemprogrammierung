/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * clientthread.h: Header für den Client-Thread
 */

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include "common/rfc.h"
#include <semaphore.h>



extern sem_t uno;



//static pthread_mutex_t zeta_mutex;
void client_thread_main(int*);

#endif
