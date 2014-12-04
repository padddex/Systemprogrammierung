/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.c: Implementierung der User-Verwaltung
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include "user.h"
#include "main.h"
#include "common/rfc.h"
#include "common/sockets.h"
#include "common/util.h"

static struct clients_info clientsInfo[MAX_PLAYER];
static pthread_mutex_t mutex;
static unsigned long verbleibende_Zeit[MAX_PLAYER];
static bool timeIsSet = false;
static unsigned long timeout[MAX_PLAYER];
static bool all_questions_answered[MAX_PLAYER];
static int player_who_anwsered_question = -1;
static int array[MAX_PLAYER];



/*
 * Überschreiben von allen Client-Informationen(Name, User-id, Punkte, Socketdescr. und Rang)
 *  und Zurücksetzen der "Spiel-Fertig"-Markierung
 */
void clearClientsInfo()
{
	/*
	 * Mutex um atomaren Ablauf zu gewährleisten
	 */
	pthread_mutex_lock(&mutex);
	/*
	 * Setzen der Client-Informationen auf 99, um klare Fallunterscheidung zwischen gültigen neu angelegten
	 * Clientinformationen und veralteten zu unterscheiden
	 */
	memset(clientsInfo, 99, sizeof(clientsInfo));
	/*
	 * Zurücksetzen der "Spiel-Fertig"-Markierung, um ein sofortiges Spielende,
	 *  bei einem neu angelegten Spiel zu verhindern
	 */
	memset(all_questions_answered, false, sizeof(all_questions_answered));
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um Spieleinfo(Name und Punkte) des aufrufenden Clients zurückzusetzen
 */
void setZeroSpieleInfo(int index)
{
	pthread_mutex_lock(&mutex);
	/*
	 * Setzen des Namens auf 0
	 */
	memset(clientsInfo[index].info.name, 0, sizeof(clientsInfo[index].info.name));
	/*
	 * Zurücksetzten der Punkte des aufrufenden Clients
	 */
	clientsInfo[index].info.punkte = 0;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um den Socketdescr. des aufrufenden Clients zu erhalten
 */
int getSocketDescr(int index)
{
	/*
	 * Zurückgeben des Socketsdescr.
	 */
	return clientsInfo[index].socket_descr_client;
}

/*
 * Funktion um gesamte Clients-Informationen(Name,Punkte, User-Id,Socketdescr. und Rang)  zurückzugeben
 */
struct clients_info* getClientsInfo()
{
	return clientsInfo;
}
/*
 * Funktion um "Spiel-Fertig"-Markierung zu setzten, um zu Markieren dass alle Fragen vom Client beantwortet wurden
 * und er bereit ist in die Endphase des Spiels zu wechseln
 */
void setFertig(int index)
{
	pthread_mutex_lock(&mutex);
	infoPrint("Spieler %s mit der Id %d ist fertig", clientsInfo[index].info.name, clientsInfo[index].info.user_id);
	/*
	 * Markieren, dass alle Fragen beantwortet, an der User-Id entsprechenden Stelle im Array
	 */
	all_questions_answered[index] = true;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um User-Id eines Spielers entsprechend des übergebenen Indexes, zu setzen
 */
void setUserId(int index)
{
	pthread_mutex_lock(&mutex);
	clientsInfo[index].info.user_id = index;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um User-Id des aufrufenden Clients zu bekommen
 */
int getUserId(int index)
{
	return clientsInfo[index].info.user_id;
}
/*
 * Funtion um den Socketdescr des aufrufenden Clients entsprechend des übergebenen Parameters zu setzen
 */
void setSocketDescr(int index, int socketDescr)
{
	pthread_mutex_lock(&mutex);
	clientsInfo[index].socket_descr_client = socketDescr;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um die gebrauchte Zeit zur beantwortung einer Frage, zur Punkteberechnung zu setzen
 */
void setZeit(int index, unsigned long zeit, unsigned long tout)
{
	pthread_mutex_lock(&mutex);
	/*
	 * Umrechnen der Timeoutzeit von Sekunden in uSekunden
	 * Timeoutzeit minus der gebrauchten Zeit um die Frage zu beantworten
	 * und schreiben des Ergebnisses an die dem aufrufenden Clients entsprechenden Stelle
	 */
	verbleibende_Zeit[index] = (tout*1000)-zeit;
	/*
	 * Schreiben des Timeouts in uSekunden
	 */
	timeout[index] = tout*1000;
	/*
	 * Setzen der Variablen auf true, um zu Signalisieren, das Zeitdaten vorhanden sind
	 */
	timeIsSet = true;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um die Verbleibende Zeit zurückzugeben
 */
unsigned long getZeit(int index)
{
	/*
	 * Überprüfen ob überhaupt Zeitdaten vorhanen sind,wenn ja, Zeit zurückgeben, wenn nicht 0 zurückgeben
	 */
	if(timeIsSet)
	{
		return verbleibende_Zeit[index];
	}
	return 0;
}
/*
 * Funktion um den Timeout einer Frage zurückzugeben
 */
unsigned long getTimeout(int index)
{
	if(timeIsSet)
	{
		return timeout[index];
	}
	return 0;
}
/*
 * Setzen des sockaddr_storage, welcher unter anderem die IP's der Clients die verbunden sind enthält
 */
void setAddr(int index, struct sockaddr_storage Adress)
{
	pthread_mutex_lock(&mutex);
	clientsInfo[index].addr = Adress;
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um zu überprüfen ob ein ein gewünschter Name bereits verwendet wird
 */
int check_setName(int index, char name[32])
{
	pthread_mutex_lock(&mutex);
	int i;

	/*
	 * Durchlaufen jedes einzelenen Clients, um den Namen der bereits vorhanden Clients,
	 * mit dem gewünschten Namen des neu anmeldenden Clients zu vergleichen
	 */
	for(i = 0; i < MAX_PLAYER; i++)
	{
		/*
		 * Überprüfen ober der Client an der zu überprüfenden Stelle überhaupt existiert,
		 * auch nicht überprüfen an der eigenen Stelle, da dort aufjedenfall eine übereinstimmung existiert
		 */
		if(clientsInfo[i].info.user_id != 99 && i != index)
		{
			/*
			 * Name gleich, wenn nein schleife von neuem durchlaufen
			 */
			if((strcmp(name, clientsInfo[i].info.name) == 0))
			{
				/*
				 * Wenn der Name übereinstimmt senden einer Fehlermeldung an den Client der sich versucht anzumelden
				 */
				strncpy(errorMessage, "Der Name ist bereits vergeben!!!", sizeof(errorMessage));
				send_socket(clientsInfo[index].socket_descr_client, ErrorWarning, &errorMessage , ERROR);
				/*
				 * Schließen des Sockets des anfragenden Clients
				 */
				close(clientsInfo[index].socket_descr_client);
				pthread_mutex_unlock(&mutex);
				/*
				 * Löschen des Clients der versucht hat mit gleichem Namen sich anzumelden
				 */
				deletePlayer(index);
				return 0;
			}
		}
	}
	/*
	 * Bei keiner übereinstimmung, den Namen des Anfragenden Clients speichern
	 */
	strncpy (clientsInfo[index].info.name, name, sizeof(clientsInfo[index].info.name));
	pthread_mutex_unlock(&mutex);
	return 1;
}
/*
 * Funktion um freien Platz zu finden
 */
int getGamerListIndex()
{
	pthread_mutex_lock(&mutex);
	int i;

	for (i = 0; i < MAX_PLAYER; i++)
	{
		if (clientsInfo[i].info.user_id == 99)
		{
			pthread_mutex_unlock(&mutex);
			return i;
		}
	}
	pthread_mutex_unlock(&mutex);
	return -1;
}
/*
 * Funktion um Anzahl der aktuellen Spieler zurückzugeben
 */
int getNumberPlayers()
{
	pthread_mutex_lock(&mutex);
	int i, number_players = 0;

	/*
	 * Schleife höchstens der Anzahl der maximalen Spieler durhlaufen
	 */
	for(i = 0; i < MAX_PLAYER; i++)
	{
		/*
		 * Wenn in Clientsinfo eine gültige User-Id steht, Zähler für Spieler Anzahl erhöhen
		 */
		if(clientsInfo[i].info.user_id != 99)
		{
			number_players++;
		}
	}
	pthread_mutex_unlock(&mutex);
	return number_players;
}
/*
 * Funktion um den Namen des Aufrufenden Clients herraus zu finden
 */
void getName(int index, char name2[32])
{
	pthread_mutex_lock(&mutex);
	/*
	 * kopieren des Namen, des aufrufenden Clients mit der übergebenen User-Id, in eine temporäre Namensaufnahmen-Variablen
	 */
	strncpy (name2, clientsInfo[index].info.name , sizeof(name2));
	pthread_mutex_unlock(&mutex);
}

void setPlayerList(struct player_list *p)
{
	pthread_mutex_lock(&mutex);
	int i, j = 0;
	uint32_t points[MAX_PLAYER];
	uint32_t tmp;


	memset (points, 0, sizeof(points));
	memset (array, 99, sizeof(array));

	for(i = 0; i < MAX_PLAYER; i++)
	{
		p->players[i].punkte = 0;
		if(clientsInfo[i].info.user_id != 99)
		{
			points[i] = clientsInfo[i].info.punkte;
		}
	}

	for (i = 0; i < MAX_PLAYER-1; i++)
	{
		for(j = i+1; j < MAX_PLAYER; j++)
		{
			if(points[i] < points[j])
			{
				tmp = points[i];
				points[i] = points[j];
				points[j] = tmp;
			}
		}
	}

	j = 0;

	for(i = 0; i < MAX_PLAYER; i++)
	{
		if (clientsInfo[i].info.user_id != 99)
		{
			if (array[0] != i && array[1] != i && array[2] != i && array[3] != i && array[4] != i && array[5] != i)
			{
				if(points[j] == clientsInfo[i].info.punkte)
				{
					array[j] = i;
					clientsInfo[i].rang = j;
					p->players[j] = clientsInfo[i].info;
					j++;
					i = -1;
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
/*
 * Funkton um den Rang des aufrufenden Clients zurückzugeben
 */
int getRanking(int index)
{
	return clientsInfo[index].rang;
}
/*
 * Funktion um aufrufenden Client zu löschen
 */
void deletePlayer(int index)
{
	pthread_mutex_lock(&mutex);
	/*
	 * Schließen des Sockets, des aufrufenden Clients
	 */
	close(clientsInfo[index].socket_descr_client);
	/*
	 * Setzen des Socketdescr. und der User-Id auf 99,
	 *  um eindeutige Fallunterscheidung für ungültige(nicht angelegte Clients) zu erhalten
	 */
	clientsInfo[index].socket_descr_client = 99;
	clientsInfo[index].info.user_id = 99;
	/*
	 * Zurücksetzen der verbleibenden Zeit,
	 * um die korrekte Punkteberechnung für einen neu angemeldeten Clients zu ermöglich
	 */
	verbleibende_Zeit[index] = 0;
	sem_post(&uno_semaphore);
	pthread_mutex_unlock(&mutex);
}
/*
 * Funktion um festzustellen ob alle Spieler schon alle Fragen beantwortet haben
 */
int isFertig()
{
	int i;
	int anzahlSpieler = getNumberPlayers();

	for(i = 0; i < MAX_PLAYER; i++)
	{
		/*
		 * Überprüfen ob gültiger Client
		 */
		if(clientsInfo[i].info.user_id != 99)
		{
			/*
			 * Überprüfen ob der Client an der aktuellen Index-Stelle alle Fragen schon beantwortet hat
			 */
			if(all_questions_answered[i])
			{
				/*
				 * herrunterzählen des Zähler des die Clients zählt, die noch nicht alle Fragen beantwortet haben
				 */
				anzahlSpieler--;
			}
		}
	}

	/*
	 * Wenn alle Spieler fertig sind, zurückgeben von 0, wenn nicht zurückgeben von 1
	 */
	if(anzahlSpieler <= 0)
	{
		return 1;
	}
	return 0;
}
/*
 * Funktion um festzustellen welcher Client die Frage beantwortet hat
 */
void setPlayerAnwsered(int index)
{
	player_who_anwsered_question = index;
}
/*
 * Funktion um den Client zurück zugeben, der gerade die Frage beantwortet hat
 */
int getPlayerAnwsered()
{
	int tmp = player_who_anwsered_question;
	/*
	 * Zurücksetzen der variablen um bei einer neuen beantwortung durch einen anderen Client
	 * ein eindeutiges ergebniss zu erhalten
	 */
	player_who_anwsered_question = -1;
	return tmp;
}
/*
 * Funktion um die Punktzahl des aufrufenden Clients zu aktualisieren
 */
void setPunkte(int index, int punktzahl)
{
	clientsInfo[index].info.punkte = clientsInfo[index].info.punkte + punktzahl;

}
