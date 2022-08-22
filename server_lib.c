#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "raw_socket.h"
#include "kermit.h"
#include "server_lib.h"


int execute_cd(message_t* message, int socket) {
    message_t response;

    response = createMessage();

    char* path = (char*)message->data;

    fprintf(stdout, "AQUI PASSOU!\n");

    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        return 0;
    }

    fprintf(stdout, "PARIDADE MEU OVO\n");

    if (chdir(path) != 0) {
        errorHeader(&response, errno);
        verticalParity(&response);
        return 0;
    }

    fprintf(stdout, "SE PARAR AQUI MENTIU\n");

    setHeader(&response, ACK_T);

    sendMessage(socket, message, &response, 1);

    return 1;
}


int execute_mkdir(message_t* message, int* socket){
    struct stat st = {0};
    message_t response;

    response = createMessage();
    
    char* path = (char*)message->data;

    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        return 0;
    }

    if (stat(path, &st) == -1) {
        if(mkdir(path, 0700) != 0){
            errorHeader(&response, errno);
            verticalParity(&response);
            return 0;
        }
    }

    setHeader(&response, ACK_T);
    sendResponse(*socket, &response);
    return 1;
}

void execute_ls(message_t* message, int* socket){}

// TODO
void execute_get_server(){}

// TODO
void execute_put_server(){}