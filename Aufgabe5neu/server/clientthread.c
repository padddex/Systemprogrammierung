/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * clientthread.c: Implementierung des Client-Threads
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "clientthread.h"
#include "common/util.h"
#include "common/sockets.h"
#include "common/rfc.h"
#include "user.h"
#include "main.h"
#include "catalog.h"
#include "common/question.h"


sem_t uno;

void client_thread_main(int *userId_PTR)
{
	int i, max;
	char name[32];
	int socketDescr=0;
	fd_set socket;
	struct timeval zeit_select;
	struct timeval beginTimeout, endTimeout;
	struct netw_data netData;
	char tmp_filename[MAX_DATEINAME];
	char message[MAX_ERRORMESSAGE];
	int anzahl_dateien;
	int numQuestion=0;
	Question *Frage;
	struct question_result result;
	long seconds, useconds;
	unsigned long timeUsed;
	bool timeIsSet=false;
	int status_intern = SPIELPHASE;
	char ausgewDatei[MAX_DATEINAME];
	anzahl_dateien = getAnzDateien();
	struct error_ohne_head error_str;

	int userId = *userId_PTR;

	getName(userId, name);
	socketDescr = getSocketDescr(userId);

	infoPrint("Client_Thread gestartet für...");
	infoPrint("%s", name);

	beginTimeout.tv_sec = 0;
	beginTimeout.tv_usec = 0;
	endTimeout.tv_sec = 0;
	endTimeout.tv_usec = 0;

	sem_init(&uno, 0, 0);

	zeit_select.tv_sec = 0;
	zeit_select.tv_usec = 500000;
	FD_ZERO(&socket);

	if(userId == 0)//SPIELLEITER
	{
		infoPrint("##########################SPIELLEITER!!!!#############################################");
		catalogBrowser();
		anzahl_dateien = getAnzDateien();

		while(status == SPIELVORBEREITUNG)
		{
			if (!(FD_ISSET(socketDescr, &socket)))
			{
				FD_SET(socketDescr, &socket);
			}

			max = socketDescr;

			if (pselect(max+1, &socket, 0, 0, (const struct timespec*)&zeit_select, 0) == -1)
			{
				perror("select");
			}

			if (FD_ISSET(socketDescr, &socket))
			{
				if(receive_socket(socketDescr, &netData, &error_str) == ERROR)
				{
					if (error_str.subtype == ERROR)
					{
						infoPrint("(Listener.c) Error-Message: %s", error_str.message);
						close(socketDescr);
						pthread_exit(0);

					}
					else
					{
						infoPrint("(Listener.c) Error-Message: %s", error_str.message);
					}

				}

				switch(type_global)
				{
					case CatalogRequest:	infoPrint("Kataloganfrage!!!");
											for(i = 0; i < anzahl_dateien; i++)
											{
												memset(tmp_filename, 0, sizeof(tmp_filename));
												getDateien(tmp_filename, i);
												infoPrint("DATEINAME: %s", tmp_filename);
												send_socket(socketDescr, CatalogResponse, tmp_filename, 0);
											}
											send_socket(socketDescr, CatalogResponse, "", 0);
											type_global = 0;
											break;

					case CatalogChange:		setChosenDat(netData.datei);
											send_socket(socketDescr, CatalogChange, netData.datei, 0);
											type_global = 0;
											break;

					case StartGame:			if(getNumberPlayers()> 1)
											{
												catalogLoader(netData.datei);
												openSHM();
												infoPrint("SPIELSTART");
												//sem_post(&uno);
												status = SPIELPHASE;
											}
											else
											{
												strncpy(errorMessage, "Es müssen mindestens 2 Spieler angemeldet sein!", sizeof(errorMessage));
												send_socket(socketDescr, ErrorWarning, &errorMessage , WARNING);
												status = SPIELVORBEREITUNG;
											}
											type_global = 0;
											break;

					case 99:				infoPrint("Der Spielleiter hat das Spiel verlassen...");
											strncpy (message, "Der Spielleiter hat das Spiel verlassen", sizeof(message));
											quitServer(ERROR, message);
											pthread_exit(0);
											break;

					default: 				break;
				}
			}
		}
	}else //NICHT SPIELLEITER
	{
		while (status == SPIELVORBEREITUNG)
		{
			if (!(FD_ISSET(socketDescr, &socket)))
			{
				FD_SET(socketDescr, &socket);
			}

			max = socketDescr;

			if (pselect(max+1, &socket, 0, 0, (const struct timespec*)&zeit_select, 0) == -1)
			{
				perror("select");
			}

			if (FD_ISSET(socketDescr, &socket))
			{
				if(receive_socket(socketDescr, &netData, &error_str) == ERROR)
				{
					if (error_str.subtype == ERROR)
					{
						infoPrint("(Listener.c) Error-Message: %s", error_str.message);
						close(socketDescr);
						pthread_exit(0);

					}
					else
					{
						infoPrint("(Listener.c) Error-Message: %s", error_str.message);
					}
				}

				switch(type_global)
				{
					case CatalogRequest:	infoPrint("Kataloganfrage!!!");
											for(i = 0; i < anzahl_dateien; i++)
											{
												memset(tmp_filename, 0, sizeof(tmp_filename));
												getDateien(tmp_filename, i);
												infoPrint("DATEINAME: %s", tmp_filename);
												send_socket(socketDescr, CatalogResponse, tmp_filename, 0);
											}
											send_socket(socketDescr, CatalogResponse, "", 0);
											type_global = 0;
											getChosenDatei(ausgewDatei);
											send_socket(socketDescr, CatalogChange, ausgewDatei, 0);
											break;

					case 99:				infoPrint("Spieler mit UserId %d wird geloescht", userId);
											deletePlayer(userId);
											close(socketDescr);
											pthread_exit(0);
											break;

					default: 				break;
				}
			}

			if (getChosenDat(ausgewDatei))//Wenn der Spielleiter einen neuen Katalog auswählt wird er hier versendet
			{
				send_socket(socketDescr, CatalogChange, ausgewDatei, 0);
			}
		}
	}

	send_socket(socketDescr, StartGame, ausgewDatei, 0);
	sem_post(&uno_semaphore);
	while(status==SPIELPHASE && status_intern == SPIELPHASE)
	{
		if (!(FD_ISSET(socketDescr, &socket)))
		{
			FD_SET(socketDescr, &socket);
		}

		max = socketDescr;

		if (pselect(max+1, &socket, 0, 0, (const struct timespec*)&zeit_select, 0) == -1)
		{
			perror("select");
		}

		if (FD_ISSET(socketDescr, &socket))
		{
			if(receive_socket(socketDescr, &netData, &error_str) == ERROR)
			{
				if (error_str.subtype == ERROR)
				{
					infoPrint("(Listener.c) Error-Message: %s", error_str.message);
					close(socketDescr);
					pthread_exit(0);

				}
				else
				{
					infoPrint("(Listener.c) Error-Message: %s", error_str.message);
				}

			}

			switch(type_global)
			{
				case QuestionRequest:	Frage = getQuestion(numQuestion);
										numQuestion++;
										if (Frage == NULL)
										{
											send_socket(socketDescr, Questions, Frage, 0);
											setFertig(userId);
											status_intern = EXIT;
											timeIsSet = false;
											break;
										}
										send_socket(socketDescr, Questions, (QuestionMessage*)Frage, 0);
										if (gettimeofday(&beginTimeout,(struct timezone *)0) < 0)
										{
											perror ("gettimeofday ist in clientthread.c");
										}
										type_global = 0;
										timeIsSet = true;
										break;

				case QuestionAnswered:	if (gettimeofday(&endTimeout,(struct timezone *)0) < 0)
										{
											perror ("gettimeofday ist in clientthread.c");
										}
										seconds = endTimeout.tv_sec - beginTimeout.tv_sec;
										useconds = endTimeout.tv_usec - beginTimeout.tv_usec;
										if(useconds < 0)
										{
											useconds += 1000000;
											seconds--;
										}
										timeUsed = seconds*1000+useconds/1000;
										infoPrint("timeUsed: %lu", timeUsed);
										if(netData.selection == Frage->correct)
										{
											setPlayerAnwsered(userId);
											setZeit(userId, timeUsed, Frage->timeout);
											sem_post(&uno_semaphore);
										}

										result.selection = 0;
										result.correct = Frage->correct;
										send_socket(socketDescr, QuestionResult, &result, 0);
										type_global = 0;
										infoPrint("-------------------UserId: %i-----------------", userId);
										infoPrint("%s", name);
										break;

				case 99:				if(userId == 0)
										{
											infoPrint("Der Spielleiter hat das Spiel verlassen...");
											strncpy (message, "Der Spielleiter hat das Spiel verlassen", sizeof(message));
											quitServer(ERROR, message);
											pthread_exit(0);
										}else
										{
											infoPrint("Spieler mit UserId %d wird geloescht", userId);
											deletePlayer(userId);
											close(socketDescr);
											pthread_exit(0);
										}
										break;

				default: 				break;
			}
		}
		if(timeIsSet)
		{
			if(gettimeofday(&endTimeout,(struct timezone *)0) < 0)
			{
				perror ("gettimeofday ist in clientthread.c");
			}
			seconds = endTimeout.tv_sec - beginTimeout.tv_sec;
			useconds = endTimeout.tv_usec - beginTimeout.tv_usec;
			if(useconds < 0)
			{
				useconds += 1000000;
				seconds--;
			}
			timeUsed = (seconds*1000)+(useconds/1000);
			if(timeUsed/1000 >= Frage->timeout)
			{
				result.selection = 1;
				result.correct = Frage->correct;
				send_socket(socketDescr, QuestionResult, &result, 0);
				timeIsSet = false;
			}
		}
	}

	while (status_intern == EXIT)
	{
		if(isFertig())
		{
			uint8_t rank = getRanking(userId)+1;
			infoPrint("Rang: %d", rank);
			send_socket(socketDescr, GameOver, &rank, 0);
			status_intern = 0;
		}
	}

	if(userId == 0)
	{
		quitServer(0, NULL);
	}
	pthread_exit(0);
}

