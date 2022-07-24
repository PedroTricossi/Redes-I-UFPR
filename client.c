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
#include "queue.h"

// Global const
#define MAX_ARGS 4
#define LINE_LENGTH 5
// Colors
#define RESET "\033[0m"
#define KBLU "\033[1;34m"

void printCurrentDir() {
    char client_dir[128];
    getcwd(client_dir, 128);

    char* token = strrchr(client_dir, '/');
    int length = strlen(token);
    char* directory = malloc(length);
    memcpy(directory, token+1, length);

    printf(KBLU);
    printf("%s", directory);
    printf(RESET);
    printf("$ ");

    free(directory);
}

void initializeArgs(char* input, char* argv[]) {
    int index = 0;
    int size = strlen(input) - 2;
    char* aux = malloc(strlen(input) * sizeof(char));
    strcpy(aux, input);

    char* token = strtok(input, " ");
    while( token != NULL ) {
        if(index == 3) {
            if(strcmp(argv[0], "edit") == 0) {
                int c = 0;
                int i = 0;
                while(c < 3) {
                    if(aux[i] == 32) c++;
                    i++;
                }
                i++;
                c = 0;
                argv[index] = malloc((size - i) * sizeof(char));
                while(i < size) {
                    argv[index][c] = aux[i];
                    c++;
                    i++;
                }
                index++;
                break;
            }
        }
        argv[index] = malloc(strlen(token) + 1);
        token[strcspn(token, "\n")] = 0;
        strcpy(argv[index], token);
        index++;
    
      token = strtok(NULL, " ");
    }

    // Sets remaining space to null
    for(int i = index; i < MAX_ARGS; i++) {
        argv[i] = malloc(sizeof(NULL));
        argv[i] = NULL;
    }
}

void freeArgs(char* argv[]) {
    for(int index = 0; index < MAX_ARGS; index++) {
        free(argv[index]);
    }
}

void parseArgs(char* input, char* argv[]) {
    int index = 0;
    int size = strlen(input) - 2;
    char* aux = malloc(strlen(input) * sizeof(char));
    strcpy(aux, input);

    char* token = strtok(input, " ");
    while( token != NULL ) {
        if(index == 3) {
            if(strcmp(argv[0], "edit") == 0) {
                int c = 0;
                int i = 0;
                while(c < 3) {
                    if(aux[i] == 32) c++;
                    i++;
                }
                i++;
                c = 0;
                free(argv[index]);
                argv[index] = malloc((size - i) * sizeof(char));
                while(i < size) {
                    argv[index][c] = aux[i];
                    c++;
                    i++;
                }
                index++;
                break;
            }
        }
        argv[index] = realloc(argv[index], strlen(token) + 1);
        token[strcspn(token, "\n")] = 0;
        strcpy(argv[index], token);
        index++;
    
        token = strtok(NULL, " ");
    }

    // Sets remaining space to null
    for(int i = index; i < MAX_ARGS; i++) {
        argv[i] = realloc(argv[i], sizeof(NULL));
        argv[i] = NULL;
    }
}

void executeLCD(char* path) {
    if (chdir(path) != 0) {
        printf("%s: no such directory\n", path);
    }
}

void executeLMKDIR(char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if(mkdir(path, 0700) != 0){
            printf("%s: Directory not created\n", path);
        }
    } else {
        printf("%s: Directory already exists\n", path);
    }
}

void executeLLS(char* path) {
    DIR *d;
    struct dirent *dir;
    int i = 1;

    // Opens dir
    if(path == NULL)
        d = opendir(".");
    else
        d = opendir(path);
    
    // Reads dir contents
    if (d) {
        while((dir = readdir(d)) != NULL) {
            if(i % LINE_LENGTH != 0) {
                if(dir->d_type == 4) { // if is a dir
                    printf(KBLU);
                    printf("%-25s", dir->d_name);
                    printf(RESET);
                }
                else { // if is a file
                    printf("%-25s", dir->d_name);
                }
            }
            else {
                if(dir->d_type == 4) { // if is a dir
                    printf(KBLU);
                    printf("%-25s\n", dir->d_name);
                    printf(RESET);
                }
                else { // if is a file
                    printf("%-25s\n", dir->d_name);
                }
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

void printError(message_t* response) {
    int error_code = response->data[0];

    if(error_code == PERMISSION_E) {
        printf("Permission denied\n");
    }
    else if(error_code == DIR_E) {
        printf("No such directory on server-side\n");
    }
    else if(error_code == ARQ_E) {
        printf("No such file on server-side\n");
    }
    else if(error_code == LINHA_E){
        printf("No such line on the provided file\n");
    }
}

void mountCD(char* path, message_t* message) {
    int path_size = strlen(path);

    if(path_size > MAX_DATA) {
        printf("Invalid path: maximum allowed path has length %d\n", MAX_DATA);
    }
    else {
        message->data_size = path_size;
        message->sequence = 0;
        message->type = CD_T;
        
        for(int i = 0; i < path_size; i++) {
            message->data[i] = path[i];
        }

        verticalParity(message);
    }
}

void mountMKDIR(char* path, message_t* message) {
    int path_size = strlen(path);

    if(path_size > MAX_DATA) {
        printf("Invalid path: maximum allowed path has length %d\n", MAX_DATA);
    }
    else {
        message->data_size = path_size;
        message->sequence = 0;
        message->type = MKDIR_T;
        
        for(int i = 0; i < path_size; i++) {
            message->data[i] = path[i];
        }

        verticalParity(message);
    }
}

void mountLS(char* path, message_t* message) {
    if(path == NULL) {
        path = malloc(sizeof(char));
        path[0] = '.';
    }

    int path_size = strlen(path);

    if(path_size > MAX_DATA) {
        printf("Invalid path: maximum allowed path has length %d\n", MAX_DATA);
    }

    message->data_size = path_size;
    message->sequence = 0;
    message->type = LS_T;

    for(int i = 0; i < path_size; i++) {
        message->data[i] = path[i];
    }

    verticalParity(message);
}

void printLS(char* ls_response) {
    char* token = strtok(ls_response, " ");
    char type;
    int i = 1;

    while( token != NULL ) {
        type = token[0] - MARKER;
        token = token + 1;
        if(i % LINE_LENGTH != 0) {
            if(type == 4) { // if is a dir
                printf(KBLU);
                printf("%-25s", token);
                printf(RESET);
            }
            else { // if is a file
                printf("%-25s", token);
            }
        }
        else {
            if(type == 4) { // if is a dir
                printf(KBLU);
                printf("%-25s\n", token);
                printf(RESET);
            }
            else { // if is a file
                printf("%-25s\n", token);
            }
        }

        i++;
        token = strtok(NULL, " ");
    }
    printf("\n");
}

void mountExit(message_t* message) {
    message->type = EXIT_T;
}

int main() {
    char input[1024];
    char* argv[MAX_ARGS];
    int socket_id = ConexaoRawSocket("lo");
    message_t message, response;
    Queue_t* messages;

    printCurrentDir();
    fgets(input, sizeof(input), stdin);
    initializeArgs(input, argv);

    while(1) { 
        message = createMessage();
        response = createMessage();

        char* command = argv[0];
        if(strcmp(command, "cd") == 0) {
            mountCD(argv[1], &message);
            sendMessage(socket_id, &message, &response);
            if(response.type == ERRO_T) {
                printError(&response);
            }
            else {
                printf("Directory change to '%s' on server-side\n", argv[1]);
            }

            message = createMessage();
            message.type = ACK_T;

            sendResponse(socket_id, &message);
        }
        else if(strcmp(command, "lcd") == 0) {
            executeLCD(argv[1]);
        }
        else if(strcmp(command, "mkdir") == 0) {
            mountMKDIR(argv[1], &message);
            sendMessage(socket_id, &message, &response);
            if(response.type == ERRO_T) {
                printError(&response);
            }
            else {
                printf("Directory change to '%s' on server-side\n", argv[1]);
            }

            message = createMessage();
            message.type = ACK_T;

            sendResponse(socket_id, &message);
        }
        else if(strcmp(command, "lmkdir") == 0) {
            executeLMKDIR(argv[1]);
        }
        else if(strcmp(command, "ls") == 0) {
            int or_size = 1;
            int sequence = 0;
            char* ls_response = malloc(or_size);
            char* data;
            mountLS(argv[1], &message);
            while(response.type != END_T) {
                sendMessage(socket_id, &message, &response);
                if(response.type == ERRO_T) {
                    printError(&response);
                    break;
                }
                else if(response.type == LSC_T) {
                    data = (char*)response.data;
                    or_size += response.data_size;
                    ls_response = realloc(ls_response, or_size);
                    strcat(ls_response, data);
                }
                message = createMessage();
                message.type = ACK_T;
                sequence++;
                sequence = sequence % MAX_SEQ;
                message.sequence = sequence;
            }
            if(response.type == END_T) {
                sendResponse(socket_id, &message);
                printLS(ls_response);
            }
            free(ls_response);
        }
        else if(strcmp(command, "lls") == 0) {
            executeLLS(argv[1]);
        }
        else if(strcmp(command, "exit") == 0) {
            mountExit(&message);
            sendResponse(socket_id, &message);
            break;
        }
        else {
            printf("%s: command not found\n", command);
        }
    
        printCurrentDir();
        fgets(input, sizeof(input), stdin);
        parseArgs(input, argv);
    }
    
    freeArgs(argv);
    close(socket_id);

    return 0;
}