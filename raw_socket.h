#ifndef __RAWSOCKET__
#define __RAWSOCKET__

#include "kermit.h"

int ConexaoRawSocket(char *device);

int server_can_read();

int client_can_read();


void sendMessage(int socket_id, message_t* message, message_t* response, int sender);

int recvMessage(int socket_id, message_t* message, int wait_for);

#endif