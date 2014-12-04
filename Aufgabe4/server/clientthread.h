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

#include <stdint.h>
#include <pthread.h>
#include "common/rfc.h"

typedef struct _Client Client;

typedef struct _Client {
	Client *next;
	int sock;
	char name[NAME_MAX_LENGTH + 1];
	uint32_t score;
	uint8_t clientId;
} Client;

void client_spawnThread(int sock);

#endif
