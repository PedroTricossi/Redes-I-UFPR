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
#include "protocol.h"
#include "server_lib.h"

#define BUFFER 100

int execute_cd(message_t* message, int socket) {
    message_t response;
    response = createMessage();

    char* path = (char*)message->data;

    if(!checkParity(message)) {
        setHeader(&response, NACK);
        return 0;
    }

    fprintf(stdout, "PASSOU PARIDADE\n");

    if (chdir(path) != 0) {
        errorHeader(&response, ERRO);
        verticalParity(&response);
        sendMessage(socket, &response,  1);
        return 0;
    }

    setHeader(&response, OK);

    sendMessage(socket, &response,  1);

    return 1;
}


int execute_mkdir(message_t* message, int socket){
    message_t response;

    response = createMessage();

    char* dir_name = (char*)message->data;
    if(!checkParity(message)) {
        setHeader(&response, NACK);
        return 0;
    }

    if (mkdir(dir_name, 755) != 0) {
        errorHeader(&response, ERRO);
        verticalParity(&response);
        sendMessage(socket, &response,  1);
        return 0;
    }
    setHeader(&response, OK);

    sendMessage(socket, &response,  1);
    return 1;
}

void execute_ls(message_t* message, int socket){
    message_t newMessage, response, lsData;
    char *ls;
    char pTmp[BUFFER];
    FILE *fp;
    int i = 0, sequence = 0, rapido = 0, flag=0;

    response = createMessage();
    newMessage = createMessage();

    char* dir_name = (char*)message->data;
    if(!checkParity(message)) {
        setHeader(&response, NACK); //arrumar
        sendMessage(socket, &response, 1);
        return ;
    }
    ls = malloc(1027 * sizeof(char));

    strcpy(ls, "ls ");
    strcat(ls, dir_name);

    fp = popen(ls,"r");
    
    flag = fgets(pTmp, BUFFER-1, fp);

    if (flag == 0){ // erro
        setHeader(&newMessage, ERRO);
        newMessage.data[0] = DIR_E;
        verticalParity(&newMessage);
        sendMessage(socket, &newMessage, 1);
        return;
    }
    
    // envia o ok
    while(response.type != ACK) {
        printf("Vou enviar o ok\n");
        setHeader(&newMessage, OK);
        sendMessage(socket, &newMessage, 1);
        printf("Enviei o ok\n");
        // recebe ack do ok
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }
    printf("Recebi ack do ok\nVou enviar o TX\n");
    response = createMessage();
    // diz que vai comecar
    while(response.type != ACK) {
        newMessage = createMessage();
        setHeader(&newMessage, TX);
        sendMessage(socket, &newMessage, 1);
        printf("Enviei o TX\n");

        // recebe ack do tx
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }
    printf("Recebi ack do tx\n");

    // envia os dados
    lsData = createMessage();
    while (flag) {
        printf("Comecando nova msg\n");
        setHeader(&lsData, DADOS);
        lsData.sequence = sequence % MAX_SEQ;
        for (int j = 0; j < MAX_DATA; j++) {
            lsData.data[j] = pTmp[j];
        }
        verticalParity(&lsData);
        sendMessage(socket, &lsData, 1);
        // espera ack ou nack
        while (server_can_read() != 1); 
        rapido = 1; 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    
        // mandar a prox
        if(response.type == ACK) {
            printf("recebi um ack\n");
            sequence++;
            
        }
        // reenviar
        else if (response.type == NACK)
            printf("Recebi um nack\n");

        flag = fgets(pTmp, BUFFER-1, fp);
    }
    newMessage = createMessage();
    setHeader(&newMessage, FIM_TX);
    sendMessage(socket, &newMessage, 1);
    
}

// TODO
void execute_get_server(){}

// TODO
void execute_put_server(){}