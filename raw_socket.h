#ifndef __RAWSOCKET__
#define __RAWSOCKET__

#include "kermit.h"

int ConexaoRawSocket(char *device);

int server_can_read();

int client_can_read();

#endif