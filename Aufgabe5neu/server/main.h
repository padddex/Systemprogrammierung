/*
 * main.h
 *
 *  Created on: 02.12.2012
 *      Author: alex
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "common/rfc.h"
#include <semaphore.h>

sem_t uno_semaphore;


extern int stdinPipe[2];
extern int stdoutPipe[2];
extern unsigned short status;


void quitServer(int, char[MAX_ERRORMESSAGE]);

#endif /* MAIN_H_ */
