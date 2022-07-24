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
#include "queue.h"

int executeCD(message_t* message, message_t* response) {
    char* path = (char*)message->data;

    if(!checkParity(message)) {
        setHeader(response, NACK_T);
        return 0;
    }

    if (chdir(path) != 0) {
        errorHeader(response, errno);
        verticalParity(response);
        return 0;
    }

    setHeader(response, ACK_T);
    return 1;
}

int EXEC_MKDIR(message_t* message, message_t* response){
    struct stat st = {0};
    char* path = (char*)message->data;

    if(!checkParity(message)) {
        setHeader(response, NACK_T);
        return 0;
    }

    if (stat(path, &st) == -1) {
        if(mkdir(path, 0700) != 0){
            errorHeader(response, errno);
            verticalParity(response);
            return 0;
        }
    }

    setHeader(response, ACK_T);
    return 1;
}


int executeLS(message_t* message, Queue_t* responses) {
    DIR *d;
    struct dirent *dir;
    char* path = (char*)message->data;

    message_t response = createMessage();
    response.type = LSC_T;

    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        enQueue(responses, response);
        return 0;
    }

    d = opendir(path);
    // Reads dir contents
    if (d) {
        int total = 0;
        while((dir = readdir(d)) != NULL) {
            total += strlen(dir->d_name) + 2;
        }
        closedir(d);

        d = opendir(path);
        char* ls_response = malloc(total * sizeof(char));
        int len = 0;
        int i = 0;
        while((dir = readdir(d)) != NULL) {
            len = strlen(dir->d_name);
            ls_response[i] = ' ';
            i++;
            ls_response[i] = dir->d_type + MARKER;
            i++;
            for(int j = 0; j < len; j++) {
                ls_response[i] = dir->d_name[j];
                i++;
            }
        }

        int size = strlen(ls_response);
        int resp_idx = 0;
        int sequence = 0;

        
        for(int i = 1; i < size; i++) { //  Eliminates space at beggining
            if(resp_idx == (MAX_DATA - 1)) {
                response.data[resp_idx] = '\0';
                response.data_size = resp_idx;
                verticalParity(&response);
                enQueue(responses, response);

                response = createMessage();
                response.type = LSC_T;
                sequence++;
                sequence = sequence % MAX_SEQ;
                response.sequence = sequence;

                resp_idx = 0;
            }

            response.data[resp_idx] = ls_response[i];
            resp_idx++;
        }

        // Captures last response
        response.data[resp_idx] = '\0';
        response.data_size = resp_idx;
        verticalParity(&response);
        enQueue(responses, response);

        // Creates end of transmission
        response = createMessage();
        response.type = END_T;
        sequence++;
        sequence = sequence % MAX_SEQ;
        response.sequence = sequence;
        verticalParity(&response);
        enQueue(responses, response);
        
        free(ls_response);
    }
    else {
        errorHeader(&response, errno);
        verticalParity(&response);
        enQueue(responses, response);
        return 0;
    }

    return 1;
}


FILE* openFile(message_t* message, Queue_t* responses) {
    message_t response = createMessage();

    if(!checkParity(message)) {
        setHeader(&response, NACK_T);
        enQueue(responses, response);
        return NULL;
    }

    FILE *file_pointer;
    char* path = (char*)message->data;
    file_pointer = fopen(path, "r");

    if(file_pointer == NULL) {
        errorHeader(&response, errno);
        verticalParity(&response);
        enQueue(responses, response);
        return NULL;
    }

    return file_pointer;
}


int main() {
    int socket_id = ConexaoRawSocket("lo");
    
    message_t message, response, last_message, last_response;
    Queue_t* responses;
    unsigned char command;
    createFile();

    last_response = createMessage();
    last_message = createMessage();

    while(1) {
        message = createMessage();
        
        if(serverRead()) {
            printf("Server reading\n");
            recvMessage(socket_id, &message, 0);
            command = message.type;
        }
        else {
            command = 16; // Impossible command
        }

        if(command == CD_T) {
            if((last_message.parity != message.parity) || last_response.type == NACK_T) {
                response = createMessage();
                executeCD(&message, &response);
                last_message = message;
                last_response = response;
                sendResponse(socket_id, &response);
            }
            else {
                sendResponse(socket_id, &last_response);
            }
            // Receives ACK from client
            // and clear buffer for own message
            while(1) {
                if(serverRead()) {
                    for(int i = 0; i < 3; i++) {
                        recvMessage(socket_id, &message, 1);
                    }
                    if(message.type != ACK_T) {
                        sendResponse(socket_id, &last_response);
                    }
                    else {
                        last_message = message;
                        break;
                    }
                }
            }
        }
        else if(command == LS_T) {
            responses = createQueue();
            if((last_message.parity != message.parity) || last_response.type == NACK_T) {
                executeLS(&message, responses);
                last_message = message;
                last_response = responses->start->message;
                sendResponse(socket_id, &last_response);

                if(last_response.type == NACK_T || last_response.type == ERRO_T) {
                    deQueue(responses);
                    // Clears own response from buffer
                    recvMessage(socket_id, &message, 1);
                }

                while(responses->size > 0) {
                    if(serverRead()) {
                        if(responses->size != 1) {
                            recvMessage(socket_id, &message, 1);
                            recvMessage(socket_id, &message, 0);
                        }
                        else {
                            for(int i = 0; i < 3; i++) {
                                recvMessage(socket_id, &message, 1);
                            }
                            if(message.sequence == last_message.sequence) {
                                sendResponse(socket_id, &last_response);   
                            }
                        }

                        last_message = message;
                        int message_seq = message.sequence % MAX_SEQ;
                        if(message_seq != 0) message_seq--;
                        else message_seq = MAX_SEQ - 1;

                        if((message.type == ACK_T) && (message_seq == (last_response.sequence  % MAX_SEQ))) {
                            deQueue(responses);
                            if(responses->size > 0) {
                                last_response = responses->start->message;
                                sendResponse(socket_id, &last_response);
                            }
                        }
                        else {
                            sendResponse(socket_id, &last_response);
                        }
                    }
                }
            }
            else {
                sendResponse(socket_id, &last_response);
            }

            free(responses);
        }else if(command == MKDIR_T) {
            if((last_message.parity != message.parity) || last_response.type == NACK_T) {
                response = createMessage();
                EXEC_MKDIR(&message, &response);
                last_message = message;
                last_response = response;
                sendResponse(socket_id, &response);
            }
            else {
                sendResponse(socket_id, &last_response);
            }
            // Receives ACK from client
            // and clear buffer for own message
            while(1) {
                if(serverRead()) {
                    for(int i = 0; i < 3; i++) {
                        recvMessage(socket_id, &message, 1);
                    }
                    if(message.type != ACK_T) {
                        sendResponse(socket_id, &last_response);
                    }
                    else {
                        last_message = message;
                        break;
                    }
                }
            }
        }
        else if(command == EXIT_T) {
            break;
        }
        else {
            printf("Server waiting to read\n");   
        }

        sleep(2);
    }

    removeFile();
    close(socket_id);
    return 0;
}