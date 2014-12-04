/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.h: Header für die Katalogbehandlung und Loader-Steuerung
 */

#ifndef CATALOG_H
#define CATALOG_H


#include "common/rfc.h"






void catalogBrowser();
void catalogLoader(char[MAX_DATEINAME]);

void getDateien(char [MAX_DATEINAME], int);
int getAnzDateien();
void setChosenDat(char [MAX_DATEINAME]);
void getChosenDatei(char [MAX_DATEINAME]);
int getChosenDat(char [MAX_DATEINAME]);
Question *getQuestion(int);
int openSHM();
void closeSHM();

#endif
