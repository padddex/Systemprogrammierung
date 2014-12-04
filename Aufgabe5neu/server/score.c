/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Implementierung des Score-Agents
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include "score.h"
#include <string.h>
#include "common/util.h"
#include "common/sockets.h"
#include "user.h"
#include "main.h"
#include "login.h"
#include "common/rfc.h"

uint32_t scoreForTimeLeft ( unsigned long, unsigned long);


void spielstand_thread_main()
{
	int i, score_c_counter  = 0;
	struct player_list liste;
	int user_id[MAX_PLAYER];
	int socketDescr[MAX_PLAYER];
	unsigned long time[MAX_PLAYER];
	unsigned int timeout[MAX_PLAYER]; //Zeit um die Frage zu beantworten
	uint32_t punkte[MAX_PLAYER];
	static int playerwhoanque = -1;
	int anzahlSpieler;

	/*
	 * Alle Punkte auf null setzen
	 */
	memset(&punkte, 0, sizeof(punkte));

	while(1)
	{
		/*
		 * Warten auf sem_post() aus login_loop()
		 */
		sem_wait(&uno_semaphore);

		infoPrint("(score.c) Anzahl an angemeldeten Spielern: %i", getNumberPlayers());

		/*
		 * liste, den time arrray und den timeout array mit null initialisieren
		 */
		memset(&liste, 0, sizeof(liste));
		memset(&time, 0, sizeof(time));
		memset(&timeout, 0, sizeof(timeout));

		infoPrint("(score.c) Jetzt wird die Player_List gesendet...");

		/*
		 * Socketdescriptor und die UserID aus user.c holen
		 */
		for(i = 0; i < MAX_PLAYER; i++ )
		{
			socketDescr[i] = getSocketDescr(i);
			user_id[i] = getUserId(i);
			/*
			 * Wenn man sich in der Spielphase befindet noch time und timeout aus
			 * der user.c holen die in client_thread.c gesetzt wurden.
			 */
			if(status == SPIELPHASE)
			{
				time[i] = getZeit(i);
				timeout[i] = getTimeout(i);
			}
		}

		playerwhoanque = getPlayerAnwsered();
		if(playerwhoanque != -1)
		{
			setPunkte(playerwhoanque, scoreForTimeLeft(time[playerwhoanque], timeout[playerwhoanque]));
		}

		/*
		 * Anzahl der Spieler aus user.c holen
		 */
		anzahlSpieler = getNumberPlayers();
		setPlayerList(&liste);

		/*
		 * Wenn sich in der Spielphase nur noch ein Spieler befindet,
		 * wird eine Fehlermeldung versendet
		 */
		if(getNumberPlayers() < 2 && status == SPIELPHASE)
		{
			for(i = 0; i < MAX_PLAYER; i++ )
			{
				if(user_id[i] != 99)
				{
					strncpy(errorMessage, "Es haben alle ausser dir das Spiel beendet", sizeof(errorMessage));

					send_socket(socketDescr[i], ErrorWarning, &errorMessage , ERROR);
				}
			}
		}



		for(i = 0; i < MAX_PLAYER; i++ )
		{
			if(liste.players[i].user_id != 99)
			{
				liste.players[i].punkte = htonl(liste.players[i].punkte);
			}
		}
		for(i = 0; i < MAX_PLAYER; i++ )
		{
			if(user_id[i] != 99)
			{
				send_socket(socketDescr[i], PlayerList, &liste, anzahlSpieler);
			}
		}
		score_c_counter++;
		infoPrint("(score.c) Spielstand_thread ist %i mal durchgelaufen.\n",score_c_counter);
		infoPrint("--------------------------------------------------------------------------------------------------------------\n");

	}

	pthread_exit(0);
}

uint32_t scoreForTimeLeft ( unsigned long timeLeft, unsigned long timeout )
{
	infoPrint("(score.c) In Punkteberechnen");
	uint32_t score = ( timeLeft *1000) / timeout ;
	infoPrint("(score.c) Ergebnis = %lu", ( timeLeft *1000) / timeout);
	infoPrint("(score.c) score = %d", score);
	/* auf 10 er - Stellen runden */
	score = (( score + 5 ) / 10) * 10;
	return score ;
}


