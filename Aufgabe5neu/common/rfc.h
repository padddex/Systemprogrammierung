/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Gemeinsam verwendete Module
 * 
 * rfc.h: Definitionen für das Netzwerkprotokoll
 */

#ifndef RFC_H
#define RFC_H

#include "common/question.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define ERROR 1
#define NO_ERROR 2
#define WARNING 0
#define MAX_PLAYER 6
#define MAX_DATEINAME 150
#define MAX_ERRORMESSAGE 150
#define SPIELVORBEREITUNG 0
#define SPIELPHASE 1
#define EXIT 3

char errorMessage[MAX_ERRORMESSAGE];


enum type
{
	LoginRequest 		= 1,
	LoginResponseOK 	= 2,
	CatalogRequest		= 3,
	CatalogResponse 	= 4,
	CatalogChange 		= 5,
	PlayerList			= 6,
	StartGame			= 7,
	QuestionRequest		= 8,
	Questions 			= 9,
	QuestionAnswered 	= 10,
	QuestionResult		= 11,
	GameOver			= 12,
	ErrorWarning		= 255
};


#pragma pack(1)


struct rfc_head								/*Enthält Informationen über Typ und Länge des darauffolgenden Receive-Pakets*/
{
	uint8_t type;
	uint16_t length;
};

struct login_request						/*Enthält den Namen des Spielers welcher dessen Client an den Server übermittelt*/
{
	uint8_t type;
	uint16_t length;
	char name[32];

};

struct login_response_ok					/*Enthält die (durch den Server ermittelte) einzigartige ID des Spielers, die an den Client übermittelt wird*/
{
	uint8_t type;
	uint16_t length;
	uint8_t client_id;
};

struct catalog_request						/*Enthält keine Daten und dient nur zur Übermittlung der Katalog-Anfrage des Clients an den Server*/
{
	uint8_t type;
	uint16_t length;
};

struct catalog_response						/*Enthält den Dateinamen einer einzelnen Datei, die durch den Loader geladen wurde. Der Server sendet alle Dateinamen vereinzelt an den Client*/
{
	uint8_t type;
	uint16_t length;
	char filename[MAX_DATEINAME];
};

struct catalog_change						/*Enthält den Namen, des durch den Client mit der ID 0 (=Spielleiter) in der GUI ausgewählten Fragekatalogs. Wird an den Server gesendet und von dort an alle Clients übermittelt*/
{
	uint8_t type;
	uint16_t length;
	char filename[MAX_DATEINAME];
};

struct start_game							/*Enthält den Namen des Fragekatalogs den der Spielleiter aktuell ausgewählt hatte als er auf 'Start Spiel' drückte. Wird vom Client an den Server gesendet.*/
{
	uint8_t type;
	uint16_t length;
	char filename[MAX_DATEINAME];
};

struct question_request						/*Enthält keine Daten und dient nur zur Übermittlung der Aufforderung des Clients an den Server die nächste Frage zu senden*/
{
	uint8_t type;
	uint16_t length;
};

struct question_package						/*Enthält eine Frage samt 4 Antwortmöglichkeiten und Timeout-Zeit. Wird vom Server an den Client gesendet*/
{
	uint8_t type;
	uint16_t length;
	QuestionMessage data;
};

struct question_answered					/*Enthält die Nummer der Antwortmöglichkeit die der Spieler angeklickt hat. Wird vom Client zum Server gesendet*/
{
	uint8_t type;
	uint16_t length;
	uint8_t selection;
};

/*
 * Enthält Angaben darüber ob die Zeit abgelaufen ist (selection) und welche
 * Antwortmöglickeit die richtige war (correct). Wird vom Server an den Client gesendet
 */
struct question_result
{
	uint8_t type;
	uint16_t length;
	uint8_t selection;
	uint8_t correct;
};

struct question_result_without_header		/*Siehe vorherigen Kommentar (Hilfsstruktur zum einfachen Schreiben der empfangenen Daten in die in 'netw' verschachtelte Struktur 'results'*/
{
	uint8_t selection;
	uint8_t correct;
};

struct game_over							/*Enthält die Platzierung des Spielers nach Beendigung des Spiels. Wird vom Server an den Client gesendet.*/
{
	uint8_t type;
	uint16_t length;
	uint8_t rank;
};

struct error_warning						/*Enthält einen Text welcher Auskunft über die Art des 'geworfenen' Errors gibt*/
{
	uint8_t type;
	uint16_t length;
	uint8_t subtype;
	char message[150];
};

struct error_ohne_head						/*Siehe vorherigen Kommentar*/
{
	uint8_t subtype;
	char message[150];
};
#pragma pack(0)
#endif
