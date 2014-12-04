/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Gemeinsam verwendete Module
 * 
 * rfc.h: Definitionen für das Netzwerkprotokoll gemäß dem RFC
 */

#ifndef RFC_H
#define RFC_H

#include "common/question.h"

#define VERSION 6
#define ERR_MAX_LENGTH 255
#define NAME_MAX_LENGTH 31

typedef enum {
	TYPE_NONE,
	TYPE_LRQ,
	TYPE_LOK,
	TYPE_CRQ,
	TYPE_CRE,
	TYPE_CCH,
	TYPE_LST,
	TYPE_STG,
	TYPE_QRQ,
	TYPE_QUE,
	TYPE_QAN,
	TYPE_QRE,
	TYPE_GOV,
	TYPE_ERR
} PacketType;

typedef enum {
	STATUS_OK,
	STATUS_CLOSED,
	STATUS_SOCK_ERR,
	STATUS_TIMEOUT,
	STATUS_INVALID_VERSION
} PacketStatus;

#pragma pack(push,1)

typedef struct {
	uint8_t type[3];
	uint16_t length;
} PacketHeader;

typedef struct {
	PacketHeader header;
	uint8_t version;
	char name[NAME_MAX_LENGTH];
} PacketLoginRequest;

typedef struct {
	PacketHeader header;
	uint8_t version;
	uint8_t clientId;
} PacketLoginResponseOK;

typedef struct {
	char name[NAME_MAX_LENGTH + 1];
	uint32_t score;
	uint8_t clientId;
} PacketPlayer;

typedef struct {
	PacketHeader header;
	PacketPlayer players[4];
} PacketPlayerList;

typedef enum {
	ERR_ERROR = 0,
	ERR_WARNING = 1
} PacketErrSubtype;

typedef struct {
	PacketHeader header;
	uint8_t subtype;
	char message[ERR_MAX_LENGTH];
} PacketErrorWarning;

#pragma pack(pop)


int packet_loginResponseOK(int sock, uint8_t clientId);
int packet_playerList(int sock, PacketPlayer *list, size_t count);
int packet_errorWarning(int sock, PacketErrSubtype type, const char *message);
void* packet_receive(int sock, uint8_t timeout, PacketType *type, PacketStatus *status);

#endif
