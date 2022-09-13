#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "raw_socket.h"
#include "protocol.h"
#include "local_lib.h"

// Global const
#define MAX_ARGS 4
#define LINE_LENGTH 5
#define STRING_MAX_SIZE 128

 
// Colors
#define RESET "\033[0m"
#define KBLU "\033[1;34m"

void printCurrentDir() {
    char *path;
    path = malloc(STRING_MAX_SIZE * sizeof(char));

    getcwd(path, STRING_MAX_SIZE);

    printf(KBLU);
    printf("%s$ ", path);
    printf(RESET);

    free(path);
}


void execute_cd_local() {
    char *path;
    path = malloc(STRING_MAX_SIZE * sizeof(char));

    scanf("%s", path);
    if( chdir(path) )
        fprintf(stderr, "Erro na execução do comando: %s\n");
    
    free(path);
}

void execute_mkdir_local() {
    char *path;
    struct stat st = {0};

    path = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", path);

    if (stat(path, &st) == -1){
        if(mkdir(path, 0700) != 0)
            printf("%s: Directory not created\n", path);
    } else
        printf("%s: Directory already exists\n", path);
    
    free(path);
}

void execute_ls_local() {
    char *path;
    char *ls;

    path = malloc(STRING_MAX_SIZE * sizeof(char));
    ls = malloc(STRING_MAX_SIZE + 3 * sizeof(char));
    scanf("%s", path);
    strcpy(ls, "ls ");

    strcat(ls, path);

    system(ls);
}

void execute_cd_server(int socket) {
    char *path;
    int rapido = 0;
    message_t message, response;

    path = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", path);
    
    int path_size = strlen(path);

    fprintf(stdout, "PATH: %s \n", path);

    message = createMessage();
    response = createMessage();
    if(path_size > MAX_DATA) {
        printf("Invalid path: maximum allowed path has length %d\n", MAX_DATA);
    }
    else {
        message.data_size = path_size;
        message.sequence = 0;
        message.type = 7;

        for(int i = 0; i < path_size; i++) {
            message.data[i] = path[i];
        }

        verticalParity(&message);
    }

    sendMessage(socket, &message,  0);
    sleep(1);

    while (client_can_read() != 1){   
        rapido = 1; 
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    if (rapido == 0){
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    if(response.type == OK)
        printf("CD funcionou para: '%s' \n", path);
        
    else if (response.type == ERRO)
        printf("Erro na troca de diretórios \n");
}

void execute_mkdir_server(int socket){
    char *dir_name;
    int rapido = 0;
    message_t message, response;

    dir_name = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", dir_name);
    int dname_size = strlen(dir_name);

    message = createMessage();
    response = createMessage();

    if (dname_size > MAX_DATA) {
        printf("Invalid dir name: maximum allowed dir name has length %d\n", MAX_DATA);
    }
    else {
        message.data_size = dname_size;
        message.sequence = 0;
        message.type = 8;

        for(int i = 0; i < dname_size; i++) {
            message.data[i] = dir_name[i];
        }

        verticalParity(&message);
    }
    

    sendMessage(socket, &message,  0);
    sleep(1);

    while (client_can_read() != 1){   
        rapido = 1; 
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    if (rapido == 0){
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    if(response.type == OK)
        printf("Diretório: '%s' \n", dir_name);
        
    else if (response.type == ERRO)
        printf("Erro ao criar diretório \n");

}

// TODO
void execute_ls_server(int socket){
    char *dir_name;
    int rapido = 0;
    message_t message, response;

    dir_name = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", dir_name);
    int dname_size = strlen(dir_name);

    message = createMessage();
    response = createMessage();

    if (dname_size > MAX_DATA) {
        printf("Invalid dir name: maximum allowed dir name has length %d\n", MAX_DATA);
    }
    else {
        message.data_size = dname_size;
        message.sequence = 0;
        message.type = 1;

        for(int i = 0; i < dname_size; i++) {
            message.data[i] = dir_name[i];
        }

        verticalParity(&message);
    }
    
    sendMessage(socket, &message,  0);

    // Espera um ok
    while (client_can_read() != 1);

    response = createMessage();
    if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
        fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
        return;
    }

    if (response.type != OK) {
        fprintf(stdout, "Algo deu errado\n");
        return;
    }
    else {
        printf("Recebi ok\n");
        message = createMessage();
        setHeader(&message, ACK);
        sendMessage(socket, &message, 0);
        rapido = 0;
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
            printf("Recebi TX\n");
            message = createMessage();
            setHeader(&message, ACK);
            sendMessage(socket, &message, 0);
            while(1) {
                while (client_can_read() != 1);
                response = createMessage();
                if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
                    message = createMessage();
                    setHeader(&message, NACK);
                    sendMessage(socket, &message, 0);
                    
                }
                if (response.type == FIM_TX)
                    return;
                else if (response.type == DADOS) {
                    fprintf(stdout, "%s", (char *)response.data);
                    message = createMessage();
                    setHeader(&message, ACK);
                    sendMessage(socket, &message, 0);
                }
                else {
                    message = createMessage();
                    setHeader(&message, ACK);
                    sendMessage(socket, &message, 0);
                }
            }
            

        }
        
    }

    
    // response = createMessage();
    // recvMessage(socket, &response, 1);

    printf("Directory '%s' created on server-side\n", dir_name);
    return;
        
    fprintf(stderr, "ERRO!!!");

    message = createMessage();
    message.type = ACK;

    sendResponse(socket, &message);
}

// TODO
void execute_get(){}

// TODO
void execute_put(){}