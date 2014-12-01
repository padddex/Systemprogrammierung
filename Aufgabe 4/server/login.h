/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.h: Header für das Login
 */

#ifndef LOGIN_H
#define LOGIN_H

#include <stdint.h>
#include <pthread.h>
#include "clientthread.h"

#define CONNECTIONS_MAX	4
#define SERVER_ERROR_FULL 						"Server voll."
#define SERVER_ERROR_NAME_ALREADY_EXISTS 		"Ein Spieler mit gleichem Namen ist bereits angemeldet."
#define SERVER_ERROR_GAME_MASTER_LEFT 			"Der Spielleiter hat das Spiel verlassen. Das Spiel wurde abgebrochen."
#define SERVER_ERROR_TOO_FEW_PLAYERS			"Es ist nur noch ein Spieler übrig. Das Spiel wurde abgebrochen."
#define SERVER_ERROR_INVALID_VERSION			"Nicht unterstützte Client Version."
#define SERVER_ERROR_GAME_ALREADY_RUNNING		"Das Spiel läuft bereits."
#define SERVER_ERROR_TOO_FEW_PLAYERS_TO_START 	"Es müssen mindestens zwei Spieler angemeldet sein."

typedef struct {
	Client *first;
	pthread_mutex_t mutex;
} ClientList;

void server_start(uint16_t port);
int8_t server_addClient(Client *client, char *name, size_t length);
void server_removeClient(Client *client);
volatile int server_isRunning();
ClientList* server_getClients();

#endif
