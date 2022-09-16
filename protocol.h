#ifndef __PROTOCOL__
#define __PROTOCOL__

#define MAX_DATA 15
#define MAX_SEQ 256
#define BUFFER_SIZE 1000

// Marker
#define MARKER 126 // 126 == 0X7E == 01111110

// Types
enum TIPOS {CD = 6, LS = 7, MKDIR = 8, ACK = 3, NACK = 2, ERRO = 17, TX = 63, FIM_TX = 46, GET = 9, DESCRITOR = 24, DADOS = 32, PUT = 10, OK = 1};
enum ERROS {DIR_E = 'a', PERMISSION_E = 'b', DIR_ALREADY_EXIST = 'c', ARQ_ALREADY_EXIST = 'd', NO_SPACE = 'e'};

typedef struct {
    unsigned char marker;
    unsigned char data_size:6;
    unsigned char sequence:4;
    unsigned int type:6;
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

void sendMessage(int socket_id, message_t* message, int sender);

int recvMessage(int socket_id, message_t* message, int wait_for);

#endif