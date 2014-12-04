/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.c: Implementierung des Logins
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include "login.h"
#include "common/util.h"
#include "common/sockets.h"
#include "user.h"
#include "common/rfc.h"
#include "main.h"
#include <pthread.h>
#include <stdbool.h>
#include "clientthread.h"


void login_loop(int *socket_PTR)
{
	struct error_ohne_head error_str;
	/*
	 * Adressinformationen
	 */
	struct sockaddr_storage addr;
	char name[32];
	socklen_t addrlen;
	int socket_descr;
	int index;
	int userId[MAX_PLAYER];
	int socket = *socket_PTR;

	/*
	 * Größe der Struktur von Adressinformationen speichern
	 */
	addrlen = sizeof(addr);

	clearClientsInfo();
	/*
	 * Spielstatus auf Spielvorbereitung setzen
	 */
	status = SPIELVORBEREITUNG;

	while(status != EXIT)
	{
		if ((socket_descr = accept(socket, (struct sockaddr*)&addr, &addrlen)) < 0)
		{
			perror("acctept steht in login.c");
		}
		else if(status != SPIELVORBEREITUNG)
		{
				/*
				 * Namen des Spielers empfangen
				 */
				receive_socket(socket_descr, name, &error_str);

				strncpy(errorMessage, "Das Spiel hat brereits begonnen!!!", sizeof(errorMessage));

				/*
				 * Fehlermeldung an den Client senden
				 */
				send_socket(socket_descr, ErrorWarning, &errorMessage , ERROR);
				close(socket_descr);
				continue;
		}

		if((index = getGamerListIndex()) < 0)
		{
			/*
			 * Namen des Spielers empfangen
			 */
			receive_socket(socket_descr, name, &error_str);
			strncpy(errorMessage, "Es sind bereits 6 Spieler vorhanden!", sizeof(errorMessage));
			/*
			 * Fehlermeldung an Client senden
			 */
			send_socket(socket_descr, ErrorWarning, &errorMessage , ERROR);
				close(socket_descr);
		}else
		{
			/*
			 * Warte auf Clients, die zu socket connecten
			 * und öffne neuen Socket, der in socket_descr gespeichert wird.
			 */

			/*
			 * Adresse des clients in user.c setzen
			 */
			setAddr(index, addr);
			/*
			 * Socketdescriptor des clients in user.c setzen
			 */
			setSocketDescr(index, socket_descr);
			/*
			 * Punkte und name mit null initialisieren
			 */
			setZeroSpieleInfo(index);
			/*
			 * Namen des Spielers von client empfangen
			 */
			receive_socket(socket_descr, name, &error_str);
			/*
			 *check_setName() überprüft ob der Name bereits vorhanden ist
			 *und sendet eine Fehlermeldung an den client
			 */
			if(check_setName(index, name))
			{
				/*
				 * User ID in user.c setzen
				 */
				setUserId(index);
				/*
				 * User ID zuweisen die dann der clientthread bekommt
				 */
				userId[index]=index;
				pthread_t client_thread_id[MAX_PLAYER];

				/*
				 * Client Thread starten
				 */
				if((pthread_create(&client_thread_id[index], NULL, (void*)&client_thread_main, &userId[index])) != 0)
				{
					perror("Thread_create steht in main.c");
				}
				/*
				 * dem client zugewiesene ID senden
				 */
				send_socket(socket_descr, LoginResponseOK, (void*)&index, 0);
				/*
				 * den wartenden semaphore im score thread aktivieren
				 */
				sem_post(&uno_semaphore);
			}
		}
	}

	pthread_exit(0);
}
