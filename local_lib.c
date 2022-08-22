#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "raw_socket.h"
#include "kermit.h"
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
    DIR *d;
    struct dirent *dir;
    int i = 1;
    char *path;

    path = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", path);

    d = opendir(path);
    
    // Reads dir contents
    if (d) {
        while((dir = readdir(d)) != NULL) {
            if(i % LINE_LENGTH != 0) {
                printf("%-25s", dir->d_name);
            }
            else {
                printf("%-25s\n", dir->d_name);
            }
            i++;
        }
        closedir(d);
    }
    else {
        printf("%s: no such directory\n", path);
    }
    printf("\n");
}

void execute_cd_server(int socket) {
    char *path;
    message_t message, response;

    path = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", path);
    
    int path_size = strlen(path);

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

    sendMessage(socket, &message, &response, 0);

    while (client_can_read() != 1){}
    
    response = createMessage();
    recvMessage(socket, &response, 1);
    
    printf("Directory change to '%s' on server-side\n", path);
    return;
        
    fprintf(stderr, "ERRO!!!");

    message = createMessage();
    message.type = ACK_T;

    sendResponse(socket, &message);
}

void execute_mkdir_server(){}

// TODO
void execute_ls_server(){}

// TODO
void execute_get(){}

// TODO
void execute_put(){}