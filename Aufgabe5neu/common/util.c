/**
 * \file	common/util.c
 * \author	Stefan Gast
 *
 * \brief	Implementierung diverser Hilfsfunktionen für Ein-/Ausgabe, Fehlersuche usw.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"


static const char *prog_name = "<unknown>";	/**< Aufrufname des Programms (argv[0]) */
static int debug_enabled = 0;			/**< Debug-Ausgaben eingeschaltet? */


/**
 * \brief	Den Programmnamen setzen
 *
 * Setzt den Namen des Programms (normalerweise argv[0] von main).
 * Es wird kein Speicher reserviert, sondern die gleiche Adresse wie für
 * das Argument verwendet!
 *
 * \see		getProgName
 */
void setProgName(const char *argv0	/**< argv[0] von main */
		)
{
	prog_name = argv0;
}


/**
 * \brief	Den Programmnamen abfragen
 *
 * Fragt den Aufrufnamen des Programms ab, der zuvor mit setProgName
 * gesetzt wurde.
 *
 * \return	Der zuvor mit setProgName gesetzte Name, oder "<unknown>"
 * 		falls setProgName noch nicht aufgerufen wurde.
 *
 * \see		setProgName
 */
const char *getProgName(void)
{
	return prog_name;
}


/**
 * \brief	Debug-Ausgaben einschalten
 *
 * Schaltet die Debug-Ausgaben ein.
 */
void debugEnable(void)
{
	debug_enabled = 1;
}


/**
 * \brief	Prüfen, ob Debug-Meldungen aktiv sind
 *
 * \retval	0: Debug-Meldungen derzeit ausgeschaltet
 * \retval	1: Debug-Meldungen aktiv
 */
int debugEnabled(void)
{
	return debug_enabled;
}


/**
 * \brief	Debug-Ausgaben ausschalten
 *
 * Schaltet die Debug-Ausgaben aus.
 */
void debugDisable(void)
{
	debug_enabled = 0;
}


/**
 * \brief	Debug-Meldung ausgeben
 *
 * Gibt eine Meldung auf der Standardfehlerausgabe aus, falls Debug-Ausgaben aktiviert sind.
 * Der Meldung wird der Name des Programms, gefolgt von ": " vorangestellt. Jede Ausgabe wird
 * durch einen Zeilenumbruch abgeschlossen.
 *
 * \see		debugEnable, debugDisable, setProgName
 */
void debugPrint(const char *fmt,	/**< printf-Formatstring */
		...			/**< zusätzliche Argumente */
	       )
{
	va_list args;

	if(debug_enabled)
	{
		va_start(args, fmt);
		flockfile(stderr);			/* Stream sperren, sodass die Zeile immer komplett und zusammenhängend ausgegeben wird */
		fprintf(stderr, "%s: ", getProgName());
		vfprintf(stderr, fmt, args);
		putc_unlocked('\n', stderr);
		funlockfile(stderr);			/* Sperre freigeben */
		va_end(args);
	}
}


/**
 * \brief	Informations-Meldung ausgeben
 *
 * Gibt eine Meldung auf der Standardausgabe aus.
 * Der Meldung wird der Name des Programms, gefolgt von ": " vorangestellt. Jede Ausgabe wird
 * durch einen Zeilenumbruch abgeschlossen.
 *
 * \see		setProgName
 */
void infoPrint(const char *fmt,		/**< printf-Formatstring */
		...			/**< zusätzliche Argumente */
	       )
{
	va_list args;

	va_start(args, fmt);
	flockfile(stdout);			/* Stream sperren, sodass die Zeile immer komplett und zusammenhängend ausgegeben wird */
	printf("%s: ", getProgName());
	vprintf(fmt, args);
	putc_unlocked('\n', stdout);
	funlockfile(stdout);			/* Sperre freigeben */
	va_end(args);
}


/**
 * \brief	Fehlermeldung ausgeben
 *
 * Gibt eine Meldung auf der Standardfehlerausgabe aus.
 * Der Meldung wird der Name des Programms, gefolgt von ": " vorangestellt. Jede Ausgabe wird
 * durch einen Zeilenumbruch abgeschlossen.
 *
 * \see		setProgName
 */
void errorPrint(const char *fmt,	/**< printf-Formatstring */
		...			/**< zusätzliche Argumente */
	       )
{
	va_list args;

	va_start(args, fmt);
	flockfile(stderr);			/* Stream sperren, sodass die Zeile immer komplett und zusammenhängend ausgegeben wird */
	fprintf(stderr, "%s: ", getProgName());
	vfprintf(stderr, fmt, args);
	putc_unlocked('\n', stderr);
	funlockfile(stderr);			/* Sperre freigeben */
	va_end(args);
}


/**
 * \brief	Fehlermeldung in Abhängigkeit zu errno ausgeben
 *
 * Gibt eine Fehlermeldung nach dem Muster
 * "Programmname: prefix: strerror(errno)" auf der Standardfehlerausgabe aus.
 * Im Gegensatz zu strerror ist diese Funktion threadsicher.
 *
 * \see		setProgName
 */
void errnoPrint(const char *prefix	/**< Text, der der Fehlerbeschreibung vorangestellt wird */
	       )
{
	flockfile(stderr);		/* Stream sperren, sodass die Zeile immer komplett und zusammenhängend ausgegeben wird */
	fprintf(stderr, "%s: ", getProgName());
	perror(prefix);
	funlockfile(stderr);		/* Sperre freigeben */
}


/**
 * \brief	Inhalt eines Puffers als Hexdump ausgeben, falls Debug-Ausgaben aktiviert sind
 *
 * Gibt den Inhalt des übergebenen Puffers als Hexdump auf der
 * Standardfehlerausgabe aus, falls Debug-Ausgaben aktiviert sind.
 * Jede Zeile hat dabei folgenden Aufbau:
 * Programmname: Präfix: Hexdaten
 * Das Präfix wird an dieser Stelle wie bei printf zusammengesetzt.
 *
 * \see		vhexdump, hexdump, setProgName, debugEnable, debugDisable
 */
void debugHexdump(const void *ptr,		/**< Zeiger auf die auszugebenden Daten */
		  size_t n,			/**< Anzahl der auszugebenden Bytes */
		  const char *fmt,		/**< Formatstring für das Präfix */
		  ...				/**< Zusätzliche Parameter, analog zu printf */
		 )
{
	va_list args;

	if(debug_enabled)
	{
		va_start(args, fmt);
		vhexdump(ptr, n, fmt, args);
		va_end(args);
	}
}


/**
 * \brief	Inhalt eines Puffers als Hexdump ausgeben
 *
 * Gibt den Inhalt des übergebenen Puffers als Hexdump auf der
 * Standardfehlerausgabe aus. Jede Zeile hat dabei folgenden Aufbau:
 * Programmname: Präfix: Hexdaten
 * Das Präfix wird an dieser Stelle wie bei printf zusammengesetzt.
 *
 * \see		vhexdump, debugHexdump, setProgName, debugEnable, debugDisable
 */
void hexdump(const void *ptr,		/**< Zeiger auf die auszugebenden Daten */
	     size_t n,			/**< Anzahl der auszugebenden Bytes */
	     const char *fmt,		/**< Formatstring für das Präfix */
	     ...			/**< Zusätzliche Parameter, analog zu printf */
	    )
{
	va_list args;

	va_start(args, fmt);
	vhexdump(ptr, n, fmt, args);
	va_end(args);
}


/**
 * \brief	Inhalt eines Puffers als Hexdump ausgeben
 *
 * Gibt den Inhalt des übergebenen Puffers als Hexdump auf der
 * Standardfehlerausgabe aus. Jede Zeile hat dabei folgenden Aufbau:
 * Programmname: Präfix: Hexdaten
 * Das Präfix wird an dieser Stelle wie bei vprintf zusammengesetzt.
 *
 * \see		debugHexdump, hexdump, setProgName, debugEnable, debugDisable
 */
void vhexdump(const void *ptr,		/**< Zeiger auf die auszugebenden Daten */
	      size_t n,			/**< Anzahl der auszugebenden Bytes */
	      const char *fmt,		/**< Formatstring, analog zu printf */
	      va_list args		/**< Argumentliste, passend zum Formatstring */
	     )
{
	const size_t charsPerLine = 16U;
	const size_t fullLines = n/charsPerLine;
	const size_t incompleteLine = n%charsPerLine;
	const unsigned char *array = (const unsigned char *)ptr;
	char byte;
	size_t line;
	size_t column;
	va_list a;

	/* Dump soll an einem Stück erfolgen, ohne von anderen Ausgaben unterbrochen zu werden */
	flockfile(stderr);

	/* komplette Zeilen mit 16 Bytes ausgeben */
	for(line=0; line<fullLines; ++line)
	{
		/* Programmname voranstellen */
		fprintf(stderr, "%s: ", getProgName());

		/* nun das Präfix */
		va_copy(a, args);	/* Argumente kopieren, denn nach vfprintf wäre args ungültig (benötigt aber C99-Unterstützung) */
		vfprintf(stderr, fmt, a);
		va_end(a);
		fprintf(stderr, ": ");

		/* Bytes als Hexwerte ausgeben */
		for(column=0; column<charsPerLine; ++column)
			fprintf(stderr, "%02x ", (unsigned)array[line*charsPerLine + column]);

		/* Abstand einfügen */
		fprintf(stderr, "%7s", "");

		/* Bytes als ASCII-Zeichen anzeigen */
		for(column=0; column<charsPerLine; ++column)
		{
			byte = array[line*charsPerLine + column];
			fprintf(stderr, "%c ", isgraph(byte) ? byte : '.');
		}

		/* Zeile abschließen */
		putchar('\n');
	}

	/* letzte, unvollständige Zeile ausgeben */
	if(incompleteLine)
	{
		/* Programmname voranstellen */
		fprintf(stderr, "%s: ", getProgName());

		/* nun das Präfix */
		va_copy(a, args);	/* Argumente kopieren, denn nach vfprintf wäre args ungültig (benötigt aber C99-Unterstützung) */
		vfprintf(stderr, fmt, a);
		va_end(a);
		fprintf(stderr, ": ");

		/* Bytes als Hexwerte ausgeben */
		for(column=0; column<incompleteLine; ++column)
			fprintf(stderr, "%02x ", (unsigned)array[line*charsPerLine + column]);

		/* leere Hexstellen auffüllen */
		while(column++ < charsPerLine)
			fprintf(stderr, "   ");

		/* Abstand einfügen */
		fprintf(stderr, "%7s", "");

		/* Bytes als ASCII-Zeichen anzeigen */
		for(column=0; column<incompleteLine; ++column)
		{
			byte = array[line*charsPerLine + column];
			fprintf(stderr, "%c ", isgraph(byte) ? byte : '.');
		}

		/* Zeile abschließen */
		putchar('\n');
	}

	/* Sperre für Standardausgabe wieder freigeben */
	funlockfile(stderr);
}


/**
 * \brief	Eine Zeile aus einem Eingabekanal lesen
 *
 * Liest eine Zeile aus einem Eingabekanal. Der Speicher dafür wird dynamisch mit malloc
 * reserviert. Der abschließende Zeilenumbruch wird durch ein Nullbyte ersetzt.
 *
 * \attention	Die Funktion ist nicht cancel-safe, da beim Abbruch der mit malloc reservierte
 * 		Speicher verloren geht!
 *
 * \return	Die gelesene Zeile, oder NULL bei Fehler (errno wird entsprechend gesetzt)
 */
char *readLine(int fd)		/**< Der Dateideskriptor, von dem gelesen werden soll */
{
	static const size_t bufferGrow = 512;
	size_t bufferSize = 0;
	size_t bufferPos = 0;
	char *buffer = NULL;
	char *newBuffer = NULL;

	for(;;)
	{
		/* mehr Speicher reservieren, falls Puffer voll */
		if(bufferPos >= bufferSize)
		{
			bufferSize += bufferGrow;
			newBuffer = realloc(buffer, bufferSize);
			if(newBuffer == NULL)
			{
				free(buffer);
				return NULL;
			}
			buffer = newBuffer;
		}

		/* Byte lesen */
		errno = 0;
		if(read(fd, &buffer[bufferPos], 1) < 1)
		{
			free(buffer);
			return NULL;
		}

		/* Zeilenumbruch gelesen? */
		if(buffer[bufferPos] == '\n')
		{
			buffer[bufferPos] = '\0';	/* Zeilenumbruch durch Nullbyte ersetzen */
			return buffer;	/* Puffer zurückgeben und beenden */
		}

		++bufferPos;
	}

	return NULL;	/* wird nie erreicht */
}
