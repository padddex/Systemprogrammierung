/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Implementierung des Score-Agents
 */

#include "score.h"
#include "login.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
	pthread_t thread;
	pthread_mutex_t mutex;
	volatile int triggered;
} Score;

void* score_thread(void *param);

Score score;

void score_spawnThread()
{
	score.triggered = 0;
	pthread_mutex_init(&score.mutex, NULL);
	pthread_create(&score.thread, NULL, (void*(*)(void*))score_thread, NULL);
}

void score_joinThread()
{
	pthread_join(score.thread, NULL);
}

void score_trigger()
{
	pthread_mutex_lock(&score.mutex);
	score.triggered = 1;
	pthread_mutex_unlock(&score.mutex);
}

void* score_thread(void *param)
{
	int triggered = 0;
	int n = 0;
	PacketPlayer players[CONNECTIONS_MAX];
	ClientList *list;

	while(server_isRunning()) {
		pthread_mutex_lock(&score.mutex);
		triggered = score.triggered;
		score.triggered = 0;
		pthread_mutex_unlock(&score.mutex);

		if (triggered) {
			list = server_getClients();
			pthread_mutex_lock(&list->mutex);
			n = 0;
			for (Client *client = list->first; client != NULL; client = client->next) {
				strcpy(players[n].name, client->name);
				players[n].score = client->score;
				players[n].clientId = client->clientId;
				n++;
			}

			for (Client *client = list->first; client != NULL; client = client->next) {
				for (int i = 0; i < n; i++) {
					packet_playerList(client->sock, players, n);
				}
			}
			pthread_mutex_unlock(&list->mutex);
		}

		sleep(1);
	}

	return NULL;
}
