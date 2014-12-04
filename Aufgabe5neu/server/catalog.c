/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.c: Implementierung der Fragekatalog-Behandlung und Loader-Steuerung
 */

#include "common/server_loader_protocol.h"
#include "common/question.h"
#include "common/util.h"
#include "catalog.h"
#include "user.h"
#include "common/rfc.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "main.h"

static int anzahl_dateien;
char ups[30][MAX_DATEINAME];
static char ausgDatei[MAX_DATEINAME];
bool newDatei;
static int fd;
static Question *Frage;
static int shmGroesse;
int anzPlayer=0;
int anzFragen;

void catalogBrowser()
{
	char *message = "BROWSE\n";
	size_t messageLen = strlen(message);
	char *ptr;
	char readBuffer[1024];
	int i;

	anzahl_dateien = 0;

	if(write(stdinPipe[1], message, messageLen) < messageLen)
	{
	  perror("write");
	}


	read(stdoutPipe[0], readBuffer, sizeof(readBuffer));

	/* Ergebnis ausgeben */
	infoPrint("Ausgabe des Kindprozesses BROWSE: %s\n", readBuffer);



	ptr = strtok(readBuffer, "\n");  /*String zerteilen*/
	for(i = 0; ptr != NULL; i++)
	{
	  strncpy (ups[i], ptr, sizeof(ups[1]));
	  ptr = strtok(NULL, "\n");
	  anzahl_dateien++;
	}
}

void catalogLoader(char dateiname[MAX_DATEINAME])
{
	char message[MAX_DATEINAME+5];
	char readBuffer[1024];
	char *ptr;
	int i, j=0;
	memset (readBuffer, 0, sizeof(readBuffer));
	strncpy (message, "LOAD ", sizeof (message));
	strcat (message, dateiname);
	strcat (message, "\n");

	infoPrint("Message: %s", message);
	size_t messageLen = strlen(message);

	if(write(stdinPipe[1], message, messageLen) < messageLen)
	{
	  perror("write");
	}

	//Anzahl der Fragen ermitteln
	while(readBuffer[0] != 'L')
	{
		if (read(stdoutPipe[0], readBuffer, sizeof(readBuffer)) < 0)
		{
			perror("read LOADER");
		}

		for (i=0; readBuffer[i]!= '\n'; i++);
		readBuffer[i]= '\0';

		j++;
		infoPrint("%i. mal durchgelaufen", j);


		if (strcmp(readBuffer, "ERROR: CANNOT OPEN FILE") == 0)
		{
			infoPrint("%s", readBuffer);
			quitServer(ERROR, "SHARED MEMORY ERROR");


		}else if(strcmp(readBuffer, "ERROR: CANNOT READ FILE") == 0)
		{
			infoPrint("%s", readBuffer);
			quitServer(ERROR, "SHARED MEMORY ERROR");

		}else if(strcmp(readBuffer, "ERROR: INVALID CATALOG") == 0)
		{
			infoPrint("%s", readBuffer);
			quitServer(ERROR, "SHARED MEMORY ERROR");

		}else if(strcmp(readBuffer, "ERROR: CANNOT USE SHARED MEMORY") == 0)
		{
			infoPrint("%s", readBuffer);
			quitServer(ERROR, "SHARED MEMORY ERROR");

		}else if(strcmp(readBuffer, "ERROR: OUT OF MEMORY") == 0)
		{
			infoPrint("%s", readBuffer);
			quitServer(ERROR, "SHARED MEMORY ERROR");

		}else if(readBuffer[0] == 'L')
		{
			ptr = strtok(readBuffer, "=");
			ptr = strtok(NULL, "=");
			anzFragen = atoi(ptr);
			infoPrint("(catalog.c) Anzahl Fragen: %i", anzFragen);
			shmGroesse = sizeof(Question)* anzFragen;

		}else
		{
			infoPrint("Unbekannter Fehler :%s", readBuffer);
		}
	}
}

void getDateien(char par[MAX_DATEINAME], int i)
{
	size_t groesse = sizeof(char[MAX_DATEINAME]);
	strncpy (par, ups[i], groesse);
}

int getAnzDateien()
{
	return anzahl_dateien;
}

void setChosenDat(char *dat)
{
	strncpy (ausgDatei, dat, sizeof(ausgDatei));
	newDatei = true;
	anzPlayer = getNumberPlayers()-1;
}

void getChosenDatei(char dat[MAX_DATEINAME])
{
	strncpy (dat, ausgDatei, sizeof(ausgDatei));
}

int getChosenDat(char dat[MAX_DATEINAME])
{
	if(newDatei)
	{
		strncpy (dat, ausgDatei, sizeof(ausgDatei));
		if(anzPlayer<=1)
		{
			newDatei = false;
		}
		anzPlayer--;
		return 1;
	}
	return 0;
}

int openSHM()
{
	infoPrint("./catalog.c -----------------------------------------------openSHM(){");

	//Shared Memory öffnen
	fd = shm_open(SHMEM_NAME, O_RDONLY, (mode_t)0600);
	if (fd < 0)
	{
		perror("SHMEM_NAME ist nicht angelegt");
		return -1;
	}
	infoPrint("Shered Memory vom server aus geöffnet");

	//Grösse setzen
	ftruncate(fd,shmGroesse);
	infoPrint("Shered Memory Größe gesetzt");

	//In Adressraum einbinden
	Frage = mmap(NULL, shmGroesse, PROT_READ, MAP_SHARED, fd, 0);
	if(Frage == MAP_FAILED)
	{
		perror("SHMEM_NAME kann nicht in Adressraum eingebunden werden");
	}
	infoPrint("Katalog geladen!!!");

	infoPrint("./catalog.c -----------------------------------------------openSHM()}");
	return fd;
}

Question *getQuestion(int numQuestion)
{
	if(numQuestion<anzFragen)
	{
		return Frage+numQuestion;
	}
	return NULL;
}
/*
 * Shared Memory schließen/ enfernen
 */
void closeSHM()
{
	munmap(Frage, shmGroesse);
	close(fd);
	shm_unlink(SHMEM_NAME);
}

