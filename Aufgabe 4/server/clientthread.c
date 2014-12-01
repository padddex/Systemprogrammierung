#include "clientthread.h"
#include "login.h"
#include "common/util.h"
#include "common/rfc.h"
#include "catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void* client_thread(Client *client);

void client_spawnThread(int sock)
{
	Client *client = NULL;
	pthread_t thread;
	pthread_attr_t attrs;

	client = calloc(1, sizeof(Client));

	if (client == NULL) {
		perror("malloc");
		exit(1);
	}

	client->sock = sock;

	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attrs, (void*(*)(void*))client_thread, client);
}

void* client_thread(Client *client)
{
	int error;
	PacketType type;
	PacketStatus status;
	void *packet = NULL;
	int running = 1;
	int loggedIn = 0;

	while (server_isRunning() && running) {
		packet = packet_receive(client->sock, 1, &type, &status);

		if (status == STATUS_OK) {
			if (!loggedIn && type == TYPE_LRQ) {
				error = server_addClient(client, ((PacketLoginRequest*)packet)->name, ((PacketLoginRequest*)packet)->header.length - 1);
				if (error == -1) {
					packet_errorWarning(client->sock, ERR_ERROR, "Der Server ist bereits voll.");
					server_removeClient(client);
					running = 0;
				} else if (error == -2) {
					packet_errorWarning(client->sock, ERR_ERROR, "Dein Name wird bereits verwendet.");
					server_removeClient(client);
					running = 0;
				}

				if (packet_loginResponseOK(client->sock, client->clientId)) {
					loggedIn = 1;
				}

				infoPrint("Client(%d) %s hat sich erfolgreich angemeldet.", client->clientId, client->name);
			} else if (loggedIn) {
				// Spieler ist eingeloggt.
			}
		} else if (status == STATUS_TIMEOUT) {
			// Client hat nichts gesendet.
		} else {
			// Verbindung geschlossen.
			server_removeClient(client);
			running = 0;
		}

		free(packet);
	}

	close(client->sock);
	free(client);

	return NULL;
}
