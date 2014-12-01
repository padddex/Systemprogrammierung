/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.c: Implementierung des Logins
 */

#include "login.h"
#include "score.h"
#include "common/rfc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct {
	ClientList clients;
	pthread_mutex_t mutex;
	volatile uint8_t connections;
	volatile int running;
} Server;

static Server server;

static int8_t server_getNextClientId();
static int server_clientNameIsUnique(const char *name);
static void server_broadcastError(PacketErrSubtype subtype, const char *message);

void server_start(uint16_t port)
{
	int server_sock, client_sock, on=1;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	fd_set fds;
	struct timespec to;

	pthread_mutex_init(&server.clients.mutex, NULL);
	pthread_mutex_init(&server.mutex, NULL);
	server.running = 1;
	server.connections = 0;

	score_spawnThread();

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		perror("Socket konnte nicht erzeugt werden");
		exit(1);
	}

	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind fehlgeschlagen");
		exit(1);
	}

	listen(server_sock, 5);

	to.tv_sec = 1;
	to.tv_nsec = 0;
	while (server.running) {
		FD_ZERO(&fds);
		FD_SET(server_sock, &fds);

		if (pselect(server_sock + 1, &fds, NULL, NULL, &to, NULL) == -1) {
			perror("pselect");
			exit(1);
		}

		if (FD_ISSET(server_sock, &fds)) {
			client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
			client_spawnThread(client_sock);
		}
	}

	close(server_sock);
	score_joinThread();
}

int8_t server_getNextClientId()
{
	int exists = 0;

	for (int i = 0; i < CONNECTIONS_MAX; i++) {
		exists = 0;
		for (Client *curr = server.clients.first; curr != NULL; curr = curr->next) {
			if (curr->clientId == i) {
				exists = 1;
				break;
			}
		}

		if (!exists) {
			return i;
		}
	}

	return -1;
}

int server_clientNameIsUnique(const char *name)
{
	for (Client *curr = server.clients.first; curr != NULL; curr = curr->next) {
		if (strcmp(curr->name, name) == 0) {
			return 0;
		}
	}

	return 1;
}

int8_t server_addClient(Client *client, char *name, size_t length)
{
	int8_t res = -1;

	pthread_mutex_lock(&server.clients.mutex);
	pthread_mutex_lock(&server.mutex);

	if (server.connections < CONNECTIONS_MAX) {
		client->clientId = server_getNextClientId();
		strncpy(client->name, name, length);

		if (server_clientNameIsUnique(client->name)) {
			if (server.clients.first == NULL) {
				server.clients.first = client;
			} else {
				for (Client *tmp = server.clients.first; tmp != NULL; tmp = tmp->next) {
					if (tmp->next == NULL) {
						tmp->next = client;
						break;
					}
				}
			}

			res = client->clientId;
			server.connections++;
			score_trigger();
		} else {
			res = -2;
		}
	} else {
		res = -1;
	}

	pthread_mutex_unlock(&server.mutex);
	pthread_mutex_unlock(&server.clients.mutex);

	return res;
}

void server_removeClient(Client *client)
{
	Client *prev = NULL;

	pthread_mutex_lock(&server.clients.mutex);
	pthread_mutex_lock(&server.mutex);

	for (Client *curr = server.clients.first; curr != NULL; curr = curr->next) {
		if (curr->clientId == client->clientId) {
			if (prev != NULL) {
				prev->next = curr->next;
			} else {
				server.clients.first = curr->next;
			}
			server.connections--;

			if (client->clientId == 0) {
				server_broadcastError(ERR_ERROR, SERVER_ERROR_GAME_MASTER_LEFT);

				// Beende den Server.
				server.running = 0;
			} else if (server.connections < 2) {
				server_broadcastError(ERR_ERROR, SERVER_ERROR_TOO_FEW_PLAYERS);

				// Beende den Server.
				server.running = 0;
			}

			score_trigger();
			break;
		}

		prev = curr;
	}

	pthread_mutex_unlock(&server.mutex);
	pthread_mutex_unlock(&server.clients.mutex);
}

volatile int server_isRunning()
{
	return server.running != 0;
}

ClientList* server_getClients()
{
	return &server.clients;
}

void server_broadcastError(PacketErrSubtype subtype, const char *message)
{
	for (Client *curr = server.clients.first; curr != NULL; curr = curr->next) {
		packet_errorWarning(curr->sock, subtype, message);
	}
}
