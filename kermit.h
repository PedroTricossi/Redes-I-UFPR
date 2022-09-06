#ifndef __KERMIT__
#define __KERMIT__

#define MAX_DATA 15
#define MAX_SEQ 256
#define BUFFER_SIZE 1000

// Marker
#define MARKER 126 // 126 == 0X7E == 01111110

// Types
#define CD_T 6
#define LS_T 7
#define MKDIR_T 8
#define EXIT_T 6
#define ACK_T 3
#define NACK_T 2
#define LIF_T 10
#define LSC_T 11
#define CA_T 12
#define END_T 13
#define ENDL_T 14
#define ERRO_T 4

// Errors
#define DIR_E 'a'
#define PERMISSION_E 'b'
#define DIR_EXISTENCE 'c'
#define ARQ_EXISTENCE 'd'

typedef struct {
    unsigned char marker;
    unsigned char data_size:6;
    unsigned char sequence;
    unsigned int type:4;
    unsigned char data[MAX_DATA];
    unsigned char parity;
    unsigned int sender;
} message_t;

message_t createMessage();

void changeReader();

void setHeader(message_t* response, int type);

void errorHeader(message_t* response, int error);

void verticalParity(message_t* message);

int checkParity(message_t* message);

// IPC definitions
void createFile();

void removeFile();

int server_can_read();
int client_can_read();

void change_permission(char new_p);

void sendMessage(int socket_id, message_t* message, message_t* response, int sender);

int recvMessage(int socket_id, message_t* message, int wait_for);

#endif