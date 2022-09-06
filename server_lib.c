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

#define BUFFER 100

int execute_cd(message_t* message, int socket) {
    message_t response;

    response = createMessage();

    char* path = (char*)message->data;

    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        return 0;
    }

    if (chdir(path) != 0) {
        errorHeader(&response, errno);
        verticalParity(&response);
        return 0;
    }

    setHeader(&response, ACK_T);

    sendMessage(socket, message, &response, 1);

    return 1;
}


int execute_mkdir(message_t* message, int socket){
    message_t response;

    response = createMessage();

    char* dir_name = (char*)message->data;
    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        return 0;
    }

    if (mkdir(dir_name, 755) != 0) {
        errorHeader(&response, errno);
        verticalParity(&response);
        return 0;
    }

    setHeader(&response, ACK_T);

    sendMessage(socket, message, &response, 1);
    return 1;
}

void execute_ls(message_t* message, int* socket){
    message_t response;
    char *ls;
    char pTmp[BUFFER];
    FILE *fp;

    response = createMessage();

    char* dir_name = (char*)message->data;
    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        return ;
    }
    ls = malloc(1027 * sizeof(char));

    strcpy(ls, "ls ");
    strcat(ls, dir_name);

    fp = popen(ls,"r");

    if(fp != NULL){
        fgets(pTmp, BUFFER-1, fp);
    }

    // enviar pTmp para o client
}

// TODO
void execute_get_server(){}

// TODO
void execute_put_server(){}