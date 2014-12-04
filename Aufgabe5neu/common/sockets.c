#include "sockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include "client/listener.h"
#include "rfc.h"
#include "server/login.h"
#include "server/score.h"
#include "util.h"
#include "../client/gui/gui_interface.h"
#include "server/user.h"

int type_global;

void send_socket(int socket_descr, uint8_t type, void *para, int errorFlag )
{
//	pthread_mutex_lock(&eta_mutex);
	infoPrint("(send_socket)------------------------>Paket-packen");

	struct login_request request_login;
	struct login_response_ok response_ok_login;
	struct catalog_request request_catalog;
	struct catalog_response response_catalog;
	struct catalog_change change_catalog;
	//struct player_list list_player;
	struct start_game game_start;
	struct question_request request_question;
	//struct question_data data_question;
	struct question_package frage;
	struct question_answered answered_question;
	//struct question_result result_question;
	struct game_over over_game;
	struct error_warning warning_error;


	switch(type)
	{
		case LoginRequest: 		request_login.type = type;
								request_login.length = htons(strlen((char*)para));
								strncpy (request_login.name, (char*)para, sizeof(request_login.name));
								request_login.name[strlen((char*)para)]= '0';
								infoPrint("(sockets.c) send_Paket.type: LoginRequest");
								infoPrint("(sockets.c) send_Paket.length (= daten): %lu Byte", strlen((char*)para));
								infoPrint("(sockets.c) sizeof(send_Paket)+daten: %lu Byte", sizeof(struct rfc_head)+request_login.length);
								if ((send(socket_descr, (void*)&request_login, sizeof(struct rfc_head)+strlen((char*)para), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case LoginResponseOK: 	response_ok_login.type = type;
								response_ok_login.length = htons(sizeof(*(uint8_t*)para));
								response_ok_login.client_id = *(uint8_t*)para;
								infoPrint("(sockets.c) send_Paket.type: LoginResponseOK");
								infoPrint("(sockets.c) send_Paket.length: %d", ntohs(response_ok_login.length));
								infoPrint("(sockets.c) sizeof(send_Paket): %lu", sizeof(response_ok_login));
								infoPrint("(socktes.c) (*(uint8_t*)para) (Spieler_id): %i", *(uint8_t*)para);
								if ((send(socket_descr, (void*)&response_ok_login, sizeof(response_ok_login), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case CatalogRequest: 	request_catalog.type = type;
								request_catalog.length = 0;
								infoPrint("(sockets.c) send_Paket.type: CatalogRequest");
								if ((send(socket_descr, (void*)&request_catalog, sizeof(request_catalog), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case CatalogResponse: 	response_catalog.type = type;
								response_catalog.length = htons(strlen((char*)para));
								strncpy (response_catalog.filename, (char*)para, sizeof(response_catalog.filename));

								response_catalog.filename[strlen((char*)para)]= '0';
								infoPrint("(sockets.c) send_Paket.type: CatalogResponse");
								infoPrint("(sockets.c) send_Paket.length (= daten): %lu Byte", strlen((char*)para));
								infoPrint("(sockets.c) sizeof(send_Paket)+daten: %lu Byte", sizeof(struct rfc_head)+response_catalog.length);
								if ((send(socket_descr, (void*)&response_catalog, sizeof(struct rfc_head)+strlen((char*)para), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}

								break;

		case CatalogChange:		change_catalog.type = CatalogChange;
								infoPrint("Katalogauswahl vor dem Senden: %s", (char*)para);
								change_catalog.length = htons(strlen((char*)para));
								strncpy (change_catalog.filename, (char*)para, sizeof(change_catalog.filename));
								//change_catalog.filename[strlen((char*)para)]= '0';
								infoPrint("Katalogauswahl vor dem Senden: %s", change_catalog.filename);
								infoPrint("Groesse: %d", ntohs(change_catalog.length));
								if ((send(socket_descr, (void*)&change_catalog, sizeof(struct rfc_head)+strlen((char*)para), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}

								break;

		case PlayerList:		((struct player_list*)para)->length = htons(errorFlag*sizeof(struct spiele_info));
								((struct player_list*)para)->type = PlayerList;
								infoPrint("(sockets.c) send_Paket.type: PlayerList");
								//infoPrint("(sockets.c) send_Paket.length: %i",(ntohs(((struct player_list*)para)->length)));
								infoPrint("(sockets.c) number_players*sizeof(struct spiele_info)+sizeof(struct rfc_head): %lu Byte", errorFlag*sizeof(struct spiele_info)+sizeof(struct rfc_head));
								if ((send(socket_descr, para, errorFlag*sizeof(struct spiele_info)+sizeof(struct rfc_head), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case StartGame:			game_start.type = StartGame;
								game_start.length = htons(strlen((char*)para));
								strncpy (game_start.filename, (char*)para, sizeof(game_start.filename));
								infoPrint("Ausgewählter Katalog: %s", game_start.filename);
								if ((send(socket_descr, (void*)&game_start, sizeof(struct rfc_head)+strlen((char*)para), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}

								break;

		case QuestionRequest: 	request_question.type = type;
								request_question.length = 0;
								infoPrint("(sockets.c) send_Paket.type: QuestionRequest");
								if ((send(socket_descr, (void*)&request_question, sizeof(request_question), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case Questions:			if(para == NULL)
								{
									frage.type = Questions;
									frage.length = 0;

									if ((send(socket_descr, (void*)&frage, sizeof(struct rfc_head), 0)) < 0)
									{
										perror ("send ist in sockets.c");
									}
									break;
								}else
								{
									frage.length = htons(sizeof(QuestionMessage));
									frage.data = *(QuestionMessage*)para;
								}
								frage.type = Questions;
//								frage.data = *(QuestionMessage*)para;
								infoPrint("(sockets.c) send_Paket.type: Questions");
//								infoPrint("Frage: %s", frage.data.question);
//								infoPrint("Antwort: %s", frage.data.answers[0]);
//								infoPrint("Antwort: %s", frage.data.answers[1]);
//								infoPrint("Antwort: %s", frage.data.answers[2]);
//								infoPrint("Antwort: %s", frage.data.answers[3]);
								infoPrint("(sockets.c) ntohs(frage.length): %i",(ntohs(frage.length)));
								infoPrint("(sockets.c) sizeof(frage): %lu Byte", sizeof(frage));
								if ((send(socket_descr, (void*)&frage, sizeof(frage), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case QuestionAnswered:	answered_question.length = htons(sizeof(uint8_t));
								answered_question.type = QuestionAnswered;
								answered_question.selection = *(uint8_t *)para;
								infoPrint("(sockets.c) send_Paket.type: QuestionAnswered");
								infoPrint("selection: %i", answered_question.selection);
								infoPrint("(sockets.c) (ntohs(((struct question_result*)para)->length))): %i",ntohs(answered_question.length));
								infoPrint("(sockets.c) sizeof(struct question_result): %lu Byte", sizeof(answered_question));
								if ((send(socket_descr, (void*)&answered_question, sizeof(answered_question), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case QuestionResult:	((struct question_result*)para)->length = htons(sizeof(uint8_t)+sizeof(uint8_t));
								((struct question_result*)para)->type = QuestionResult;
								infoPrint("(sockets.c) send_Paket.type: QuestionResult");
								infoPrint("selection: %i", ((struct question_result*)para)->selection);
								infoPrint("correct: %i", ((struct question_result*)para)->correct);
								infoPrint("(sockets.c) (ntohs(((struct question_result*)para)->length))): %i",(ntohs(((struct question_result*)para)->length)));
								infoPrint("(sockets.c) sizeof(struct question_result): %lu Byte", sizeof(struct question_result));
								if ((send(socket_descr, para, sizeof(struct question_result), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case GameOver: 			over_game.type = type;
								over_game.length = htons(sizeof(uint8_t));
								over_game.rank = *(uint8_t*)para;
								infoPrint("(sockets.c) send_Paket.type: GameOver");
								infoPrint("(sockets.c) send_Paket.length: %d", ntohs(over_game.length));
								infoPrint("(sockets.c) sizeof(send_Paket): %lu", sizeof(over_game));
//								infoPrint("(socktes.c) (*(uint8_t*)para) (Spieler_id): %i", *(uint8_t*)para);
								if ((send(socket_descr, (void*)&over_game, sizeof(over_game), 0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		case ErrorWarning:      warning_error.type = type;
								warning_error.subtype = errorFlag;
								strncpy (warning_error.message, (char*)para, sizeof(warning_error.message));
								warning_error.length = htons(strlen(warning_error.message)+(sizeof(uint8_t)));
								infoPrint("(sockets.c) send_Paket.type: ErrorWarning");
								infoPrint("(sockets.c) warning_error.length (= daten): %d Byte", warning_error.length);
								infoPrint("(sockets.c) Message: %s", warning_error.message);
								infoPrint("(sockets.c) sizeof(send_Paket+daten): %lu Byte", sizeof(struct rfc_head)+sizeof(uint8_t)+warning_error.length);
								if ((send(socket_descr, (void*)&warning_error, sizeof(struct rfc_head)+sizeof(uint8_t)+strlen(warning_error.message),0)) < 0)
								{
									perror ("send ist in sockets.c");
								}
								break;

		default: 				infoPrint("(sockets.c) send_Paket.type unbekannter Typ!");

			 	 	 	 	 	break;
	}
	infoPrint("(send_socket)--------------------->Paket-versendet\n");
//	pthread_mutex_unlock(&eta_mutex);
}

int receive_socket(int socket_descr, void *ptr, struct error_ohne_head* error)
{
	//pthread_mutex_lock(&epsilon_mutex);
	struct rfc_head receive_head;
	int recvRes;


	infoPrint("(receive_socket)--------------------->Paket-packen");

	recvRes = (recv(socket_descr, (void*)&receive_head, 3, 0));

	if (recvRes < 0)
	{
		perror("recv ist in sockets.c");
	}else
	if (recvRes == 0)
	{
		type_global = 99;
		return NO_ERROR;
	}else
	{
		receive_head.length = ntohs(receive_head.length);

		switch(receive_head.type)
		{
			case LoginRequest: 		if ((recv(socket_descr, ptr, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									((char*)ptr)[receive_head.length] = 0;
									infoPrint("(sockets.c) receive_Paket.type: LoginRequest");
									infoPrint("(sockets.c) receive_Paket.data = Name: %s", (char*)ptr);
									infoPrint("(sockets.c) receive_Paket.length: %i Byte", receive_head.length);
									infoPrint("(sockets.c) sizeof(receive_Paket+length): %lu Byte", sizeof(receive_head)+(receive_head.length));
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case LoginResponseOK: 	if ((recv(socket_descr, ptr, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									infoPrint("(sockets.c) receive_Paket.type: LoginResponseOK");
									infoPrint("(sockets.c) *ptr = **Id**: %d", *(uint8_t*)ptr);
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(sockets.c) sizeof(receive_Paket): %lu Byte",sizeof(((uint8_t*) ptr)));
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case CatalogRequest: 	infoPrint("(sockets.c) CatalogReq angekommen!");
									infoPrint("(sockets.c) receive_Paket.type = %d",receive_head.type);
									infoPrint("(sockets.c) receive_Paket.length = %d",receive_head.length);
									type_global = receive_head.type;
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case CatalogResponse:	if ((recv(socket_descr, ((struct netw_data*)ptr)->datei, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									type_global = receive_head.type;
									infoPrint("Katalogauswahl empfangen!!!!");
									((struct netw_data*)ptr)->datei[receive_head.length] = 0;
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;


			case CatalogChange:		if ((recv(socket_descr, ((struct netw_data*)ptr)->datei, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									type_global = receive_head.type;
									((struct netw_data*)ptr)->datei[receive_head.length] = 0;
									infoPrint("Katalogauswahl empfangen: %s", (char*)ptr);
									infoPrint("receive_head.length: %d", receive_head.length);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case PlayerList:		memset(((struct netw_data*)ptr)->spielerliste, 99, sizeof(((struct netw_data*)ptr)->spielerliste));
									infoPrint("Laenge: %d", receive_head.length);
									if ((recv(socket_descr, ((struct netw_data*)ptr)->spielerliste, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									type_global = receive_head.type;
									infoPrint("(sockets.c) receive_Paket.type: PlayerList");
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case StartGame:			if ((recv(socket_descr, ((struct netw_data*)ptr)->datei, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									infoPrint("SPIELSTART");
									type_global = receive_head.type;
									((struct netw_data*)ptr)->datei[receive_head.length] = 0;
									infoPrint("Katalogauswahl empfangen: %s", (char*)ptr);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case QuestionRequest: 	infoPrint("(sockets.c) QuestionRequest angekommen!");
									infoPrint("(sockets.c) receive_Paket.type = %d",receive_head.type);
									infoPrint("(sockets.c) receive_Paket.length = %d",receive_head.length);
									*(int*)ptr = 1;
									type_global = receive_head.type;
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case Questions:			if(receive_head.length != 0)
									{
										if ((recv(socket_descr, &((struct netw_data*)ptr)->Fragen, receive_head.length, 0)) < 0)
										{
											perror("recv ist in sockets.c");
										}
									}else
									{
										memset(&((struct netw_data*)ptr)->Fragen, 0, sizeof(((struct netw_data*)ptr)->Fragen));
									}
									type_global = receive_head.type;
									infoPrint("(sockets.c) receive_Paket.type: Questions");
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case QuestionAnswered:	if ((recv(socket_descr, &((struct netw_data*)ptr)->selection, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									type_global = receive_head.type;
									infoPrint("(sockets.c) receive_Paket.type: QuestionAnswered");
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(sockets.c) receive_Paket.selection: %d", ((struct netw_data*)ptr)->selection);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case QuestionResult:	if ((recv(socket_descr, &((struct netw_data*)ptr)->results, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									type_global = receive_head.type;
									infoPrint("(sockets.c) receive_Paket.type: QuestionResult");
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(sockets.c) receive_Paket.selection: %d", ((struct netw_data*)ptr)->results.selection);
									infoPrint("(sockets.c) receive_Paket.correct: %d", ((struct netw_data*)ptr)->results.correct);
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case GameOver:		 	if ((recv(socket_descr, &((struct netw_data*)ptr)->rang, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									infoPrint("(sockets.c) receive_Paket.type: GameOver");
									infoPrint("(sockets.c) Rang: %d", ((struct netw_data*)ptr)->rang);
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("(sockets.c) sizeof(receive_Paket): %lu Byte",sizeof(uint8_t));
									infoPrint("(receive_socket)------------------>Paket-empfangen\n");
									type_global = receive_head.type;
//									pthread_mutex_unlock(&epsilon_mutex);
									return NO_ERROR;
									break;

			case ErrorWarning:		if ((recv(socket_descr, error, receive_head.length, 0)) < 0)
									{
										perror("recv ist in sockets.c");
									}
									infoPrint("(sockets.c) receive_Paket.type: ErrorWarning");
									infoPrint("(sockets.c) receive_Paket.length: %d Byte", receive_head.length);
									infoPrint("Error: %s", error->message);
									infoPrint("(receive_socket)------------>Fehler beim Empfangen\n");
//									pthread_mutex_unlock(&epsilon_mutex);
									return ERROR;
									break;

			case 0:					type_global = receive_head.type;
									break;

			default: 				infoPrint("(sockets.c) receive_Paket.type unbekannter Typ: %d", receive_head.type);
									exit(0);
									break;
		}

	}
	infoPrint("(receive_socket)------------>Fehler beim Empfangen\n");
//	pthread_mutex_unlock(&epsilon_mutex);
	return ERROR;
}


