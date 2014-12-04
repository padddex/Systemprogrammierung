/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Gemeinsam verwendete Module
 * 
 * rfc.c: Implementierung der Funktionen zum Senden und Empfangen von
 * Datenpaketen gemäß dem RFC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include "common/util.h"
#include "rfc.h"

#define TYPE_EQ(TYPE1, TYPE2)		(TYPE1[0] == TYPE2[0] && TYPE1[1] == TYPE2[1] && TYPE1[2] == TYPE2[2])

static int packet_send(int sock, void *data, const char *type, uint16_t length);


int packet_send(int sock, void *data, const char *type, uint16_t length)
{
	PacketHeader *header = data;

	memcpy(header->type, type, 3);
	header->length = htons(length);

	return send(sock, data, sizeof(PacketHeader) + length, 0);
}

int packet_loginResponseOK(int sock, uint8_t clientId)
{
	PacketLoginResponseOK lok;

	lok.version = VERSION;
	lok.clientId = clientId;

	return packet_send(sock, &lok, "LOK", 2);
}

int packet_playerList(int sock, PacketPlayer *list, size_t count)
{
	PacketPlayerList lst;

	for (int i = 0; i < count; i++) {
		memcpy(lst.players[i].name, list[i].name, NAME_MAX_LENGTH);
		lst.players[i].clientId = list[i].clientId;
		lst.players[i].score = htonl(list[i].score);
	}

	return packet_send(sock, &lst, "LST", sizeof(PacketPlayer) * count);
}

int packet_errorWarning(int sock, PacketErrSubtype type, const char *message)
{
	PacketErrorWarning err;

	err.subtype = type;
	memcpy(err.message, message, strlen(message));

	return packet_send(sock, &err, "ERR", strlen(message) + 1);
}

void* packet_receive(int sock, uint8_t timeout, PacketType *type, PacketStatus *status)
{
	PacketHeader *header = NULL;
	uint8_t *data = NULL;
	fd_set fds;
	struct timespec to;
	size_t length , pos;
	int n = 0;

	*type = TYPE_NONE;
	*status = STATUS_OK;

	to.tv_sec = timeout;
	to.tv_nsec = 0;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	if (pselect(sock + 1, &fds, NULL, NULL, &to, NULL) == -1) {
		perror("pselect");
		exit(1);
	}

	if (FD_ISSET(sock, &fds)) {
		if (ioctl(sock, FIONREAD, &n) < 0) {
			perror("ioctl");
			exit(1);
		}

		if (n <= 0) {
			*status = STATUS_CLOSED;
			return NULL;
		}

		data = malloc(sizeof(PacketHeader));

		if ((read(sock, data, sizeof(PacketHeader))) <= 0) {
			*status = STATUS_SOCK_ERR;
			free(data);
			return NULL;
		}

		header = (PacketHeader*)data;
		header->length = ntohs(header->length);

		if (TYPE_EQ(header->type, "LRQ")) {
				*type = TYPE_LRQ;
			} else if (TYPE_EQ(header->type, "LOK")) {
				*type = TYPE_LOK;
			} else if (TYPE_EQ(header->type, "CRQ")) {
				*type = TYPE_CRQ;
			} else if (TYPE_EQ(header->type, "CRE")) {
				*type = TYPE_CRE;
			} else if (TYPE_EQ(header->type, "CCH")) {
				*type = TYPE_CCH;
			} else if (TYPE_EQ(header->type, "LST")) {
				*type = TYPE_LST;
			} else if (TYPE_EQ(header->type, "STG")) {
				*type = TYPE_STG;
			} else if (TYPE_EQ(header->type, "QRQ")) {
				*type = TYPE_QRQ;
			} else if (TYPE_EQ(header->type, "QUE")) {
				*type = TYPE_QUE;
			} else if (TYPE_EQ(header->type, "QAN")) {
				*type = TYPE_QAN;
			} else if (TYPE_EQ(header->type, "QRE")) {
				*type = TYPE_QRE;
			} else if (TYPE_EQ(header->type, "GOV")) {
				*type = TYPE_GOV;
			} else if (TYPE_EQ(header->type, "ERR")) {
				*type = TYPE_ERR;
			} else {
				*type = TYPE_NONE;
		}

		if (*type != TYPE_NONE) {
			data = realloc(data, sizeof(PacketHeader) + header->length);
			header = (PacketHeader*)data;

			length = 0;
			pos = 0;
			while (pos < header->length && (length = read(sock, data + sizeof(PacketHeader) + pos, header->length - pos)) > 0) {
				pos += length;
			}

			if (pos != header->length) {
				*type = TYPE_NONE;
				*status = STATUS_SOCK_ERR;
				free(data);
				data = NULL;
				return NULL;
			}

			if (*type == TYPE_LST) {
				for (int i = 0; i < header->length / sizeof(PacketPlayer); i++) {
					((PacketPlayerList*)data)->players[i].score = ntohl(((PacketPlayerList*)data)->players[i].score);
				}
			} else if (*type == TYPE_LRQ || *type == TYPE_LOK) {
				if (((PacketLoginResponseOK*)data)->version != VERSION) {
					*status = STATUS_INVALID_VERSION;
					free(data);
					data = NULL;
				}
			}
		} else {
			*status = STATUS_SOCK_ERR;
			free(data);
			data = NULL;
		}
	} else {
		*status = STATUS_TIMEOUT;
	}

	return data;
}
