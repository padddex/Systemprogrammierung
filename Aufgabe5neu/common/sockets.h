#ifndef SOCKETS_H
#define SOCKETS_H


#include <stdint.h>
#include "rfc.h"
#include "server/user.h"


struct netw_data
{
	struct spiele_info spielerliste[MAX_PLAYER];
	char datei[MAX_DATEINAME];
	struct question_result_without_header results;
	uint8_t selection;
	QuestionMessage Fragen;
	uint8_t rang;
};

extern int type_global;
void send_socket(int, uint8_t, void*, int);
int receive_socket(int, void*, struct error_ohne_head*);


#endif
