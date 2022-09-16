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
#include <sys/vfs.h>

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
void execute_get_server(message_t *message, int socket){
    message_t newMessage, response, getData;
    FILE *fp;
    int i = 0, sequence = 0, flag = 0;

    response = createMessage();
    newMessage = createMessage();

    char* file_name = (char*)message->data;
    //printf("File name %s\n", file_name);
    if(!checkParity(message)) {
        setHeader(&response, NACK); //arrumar
        sendMessage(socket, &response, 1);
        return ;
    }

    fp = fopen(file_name,"r");
    
    if (!fp){ // erro
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
   
    getData = createMessage();
    getData.sequence = sequence % MAX_SEQ;
    setHeader(&getData, DADOS);
    getData.data_size = MAX_DATA;
    flag = fread(getData.data, MAX_DATA, 1, fp);
    while (flag == 1) {
        verticalParity(&getData);
        sendMessage(socket, &getData, 1);
        // espera ack ou nack
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    
        // mandar a prox
        if(response.type == ACK) {
            sequence++;
            getData = createMessage();
            getData.data_size = MAX_DATA;
            flag = fread(getData.data, MAX_DATA, 1, fp);
            getData.sequence = sequence % MAX_SEQ;
            setHeader(&getData, DADOS);
        }
    }

    for (i = 0; i < MAX_DATA; i++){
        if (getData.data[i] == 0) {
            break;
        }
    }
    
    response = createMessage();
    verticalParity(&getData);
    getData.data_size = i;
    // espera ack
    while (response.type != ACK) {
        sendMessage(socket, &getData, 1);
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    newMessage = createMessage();
    setHeader(&newMessage, FIM_TX);
    sendMessage(socket, &newMessage, 1);
    return;
    
}

// TODO
void execute_put_server(message_t *message, int socket) {
    message_t newMessage, response;
    char *file_name;
    FILE *fp;
    struct statfs st;
    int size = 0;
    response = createMessage();
    newMessage = createMessage();

    file_name = (char*)message->data;
    printf("File name %s\n", file_name);
    if(!checkParity(message)) {
        setHeader(&response, NACK); //arrumar
        sendMessage(socket, &response, 1);
        return ;
    }

    // envia o ok
    while(response.type != ACK) {
        setHeader(&newMessage, OK);
        sendMessage(socket, &newMessage, 1);
        // recebe ack do ok
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    // recebe tam do arquivo
    if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
        fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
        return;
    }
    while (response.type != PUT || !checkParity(&response)) { // recebe tam
        setHeader(&newMessage, NACK);
        sendMessage(socket, &newMessage, 1);
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    // envia ack ou erro do tamanho
    size = response.data[0];
    statfs("/dev/sda4", &st);
     if (size < st.f_bavail * 1024) {
        setHeader(&newMessage, ACK);
        sendMessage(socket, &newMessage, 1);
    } 
    else {
        setHeader(&newMessage, ERRO);
        newMessage.data[0] = NO_SPACE;
        sendMessage(socket, &newMessage, 1);
        return;
    }

    fp = fopen("novoPut", "w");

    // espera o comeco da transmissao
    while (client_can_read() != 1);
    response = createMessage();
    if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
        fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
        return;
    }
    if (response.type != TX){
        fprintf(stderr, "Algo deu errado\n");
        return;
    }
    else {
        // recebe dados
        newMessage = createMessage();
        setHeader(&newMessage, ACK);
        sendMessage(socket, &newMessage, 0);
        while(1) {
            while (client_can_read() != 1);
            response = createMessage();
            if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){ //timeout
                newMessage = createMessage();
                setHeader(&newMessage, NACK);
                sendMessage(socket, &newMessage, 0);
                
            }
            if (!checkParity(&response)){ // verifica paridade
                setHeader(&newMessage, NACK);
                sendMessage(socket, &newMessage, 0);
            }
            
            if (response.type == FIM_TX) { // se for fim de arquivo, encerra
                printf("Arquivo recebido.\n");
                fclose(fp);
                return;
            }
            else if (response.type == DADOS) { // se for dado, salva no arquivo
                for (int j = 0; j < response.data_size; j++) {
                    //if (response.data[j] != NULL) {
                        fwrite(&response.data[j], 1, 1, fp);
                    //}
                    
                    
                }
                newMessage = createMessage();
                setHeader(&newMessage, ACK);
                sendMessage(socket, &newMessage, 0);
            }
            else { // lixo
                newMessage = createMessage();
                setHeader(&newMessage, ACK);
                sendMessage(socket, &newMessage, 0);
            }
        }
        
    }

    fclose(fp);

}