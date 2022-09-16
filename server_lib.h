#ifndef __SERVERLIB__
#define __SERVERLIB__

// DONE
int execute_cd(message_t* message, int socket);

// TODO
void execute_ls(message_t* message, int socket);

// DONE
int execute_mkdir(message_t* message, int socket);

// TODO
void execute_get_server(message_t *message, int socket);

// TODO
void execute_put_server();

#endif