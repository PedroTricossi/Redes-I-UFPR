#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "protocol.h"

#define PATH "/home/carlos/UFPR/redes/"

#define ONE 0b100
#define ZERO 0b000

message_t createMessage() {
    message_t message;

    message.marker = MARKER;
    message.data_size = 0;
    message.sequence = 0;
    message.type = 0;
    message.sender = 0;
    for(int i = 0; i < MAX_DATA; i++) {
        message.data[i] = 0;
    }
    message.parity = 0;

    return message;
}

void setHeader(message_t* response, int type) {
    response->type = type;
}

void errorHeader(message_t* response, int error) {
    response->type = ERRO; // ERROR
    response->data_size = 1;
    response->data[0] = error;
}

void verticalParity(message_t* message) {
    int size = message->data_size;
    unsigned char parity;

    parity = ONE ^ ZERO;
    for(int i = 0; i < size; i++) {
        parity ^= message->data[i];
    }
    
    message->parity = parity;
}

int checkParity(message_t* message) {
    int size = message->data_size;
    unsigned char parity;

    parity = ONE ^ ZERO;
    for(int i = 0; i < size; i++) {
        parity ^= message->data[i];
    }

    
    if(message->parity == parity)
        return 1;
    
    return 0;
}

// Interprocess Communication
void createFile() {
    FILE *file_pointer;
    file_pointer = fopen("/home/carlos/UFPR/redes/Redes-I-UFPRreadPermission.txt", "w");
    fprintf(file_pointer, "42");
    fclose(file_pointer);
}

void removeFile() {
    remove("/home/carlos/UFPR/redes/Redes-I-UFPRtemporaryIPC.txt");
}

int server_can_read() {
    FILE *file_pointer;
    file_pointer = fopen("/home/carlos/UFPR/redes/Redes-I-UFPRreadPermission.txt", "r");

    char permission = fgetc(file_pointer);
    fclose(file_pointer);

    if(permission == 's') {
        return 1;    
    }

    return 0;
}

int client_can_read() {
    FILE *file_pointer;
    file_pointer = fopen("/home/carlos/UFPR/redes/Redes-I-UFPRreadPermission.txt", "r");

    char permission = fgetc(file_pointer);
    fclose(file_pointer);

    if(permission == 'c') {
        return 1;    
    }

    return 0;
}

void change_permission(char new_p) {
    FILE *file_pointer;
    file_pointer = fopen("/home/carlos/UFPR/redes/Redes-I-UFPRreadPermission.txt", "w");
    fprintf(file_pointer, "%s", &new_p);
    fclose(file_pointer);
}

void sendMessage(int socket_id, message_t* message, int sender) {
  message->sender = sender;
  if(write(socket_id, message, sizeof(*message)) == -1) {
    perror("Send failed");
  }

  if(sender == 0){
    change_permission('s');
  } else if (sender == 1){
    change_permission('c');
  }
  // organiza file descriptor para timeout
  struct pollfd fd;
  fd.fd = socket_id;
  fd.events = POLLIN;

  if( poll(&fd, 1, 1))
    read(socket_id, message, sizeof(*message));
}

int recvMessage(int socket_id, message_t* message, int wait_for) {
  // organiza file descriptor para timeout
  struct pollfd fd;
  fd.fd = socket_id;
  fd.events = POLLIN;
  
  // espera algum pacote, caso demore mais que TIMEOUT segundos, retorna 2
  int retorno = poll(&fd, 1, 5*1000);
  
  if( retorno == 0 )
    return 2;
  else if( retorno < 0 )
    return(-1);

  if(read(socket_id, message, sizeof(*message)) == -1) {
    perror("Received failed");
  }

  if (message->sender == wait_for){
    message->sender = 8;
    change_permission('p');
    return 0;
  }
  else
    recvMessage(socket_id, message, wait_for); 
}
