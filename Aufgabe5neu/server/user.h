/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.h: Header für die User-Verwaltung
 */

#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common/rfc.h"


#pragma pack(1)

struct spiele_info
{
	char name[32];
	uint32_t punkte;
	uint8_t user_id;
};

struct clients_info
{
	struct spiele_info info;
	struct sockaddr_storage addr;
	int socket_descr_client;
	uint8_t rang;
};

struct player_list
{
	uint8_t type;
	uint16_t length;
	struct spiele_info players[MAX_PLAYER];
};

#pragma pack(0)


struct clients_info* getClientsInfo();
void clearClientsInfo();
void setZeroSpieleInfo(int);
int getSocketDescr(int);
void setUserId(int);
int getUserId(int);
void getName(int, char [32]);
void setSocketDescr(int, int);
void setAddr(int, struct sockaddr_storage);
int check_setName(int, char [32]);
int getGamerListIndex();
void setFertig(int);
int isFertig();
int getRanking(int);
int getNumberPlayers();
void setPlayerList(struct player_list*);
void deletePlayer(int);
void setZeit(int, unsigned long, unsigned long);
unsigned long getTimeout(int);
unsigned long getZeit(int);
void setPlayerAnwsered(int);
int getPlayerAnwsered();
void setPunkte(int, int);


#endif
