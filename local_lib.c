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
    while (response.type != OK) {
        sendMessage(socket, &message,  0);

        // Espera um ok
        while (client_can_read() != 1);

        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }

        if (response.type == ERRO){
            checkParity(&response);
            if (response.data[0] == DIR_E) {
                printf("O diretório informado não existe\n");
                   return;
            }
        }

    }
    printf("Recebi ok\n");
    message = createMessage();
    setHeader(&message, ACK);
    sendMessage(socket, &message, 0);
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
        
    // response = createMessage();
    // recvMessage(socket, &response, 1);

    printf("Directory '%s' created on server-side\n", dir_name);
    return;
        
    fprintf(stderr, "ERRO!!!");

    message = createMessage();
    message.type = ACK;

    sendResponse(socket, &message);
}

// DOING
void execute_get(int socket){
    char *file_name;
    message_t message, response;
    FILE *fp;

    file_name = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", file_name);
    int fname_size = strlen(file_name);

    message = createMessage();
    response = createMessage();

    if (fname_size > MAX_DATA) {
        printf("Invalid file name: maximum allowed file name has length %d\n", MAX_DATA);
    }
    else {
        message.data_size = fname_size;
        message.sequence = 0;
        message.type = GET;

        for(int i = 0; i < fname_size; i++) {
            message.data[i] = file_name[i];
        }

        verticalParity(&message);
    }

    fp = fopen("teste123", "w");
    if (!fp)
        return;

    // envia o get e nome do arquivo
    while (response.type != OK) {
        sendMessage(socket, &message,  0);
        // Espera um ok
        while (client_can_read() != 1);
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
        if (response.type == ERRO){
            checkParity(&response);
            if (response.data[0] == DIR_E) {
                printf("O arquivo informado não existe\n");
                   return;
            }
        }

    }
    message = createMessage();
    setHeader(&message, ACK);
    sendMessage(socket, &message, 0);
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
            // verifica paridade e manda nack se tiver errado
            if (!checkParity(&response)){
                setHeader(&message, NACK);
                sendMessage(socket, &message, 0);
            }
            // verifica se é fim de tx
            if (response.type == FIM_TX) {
                printf("Arquivo recebido.\n");
                fclose(fp);
                return;
            }
            // se for dados, grava no arquivo
            else if (response.type == DADOS) {
                for (int j = 0; j < response.data_size; j++) {
                    //if (response.data[j] != 0) {
                        fwrite(&response.data[j], 1, 1, fp);
                    //}
                    
                }
                
                message = createMessage();
                setHeader(&message, ACK);
                sendMessage(socket, &message, 0);
            }
            else { // lixo
                message = createMessage();
                setHeader(&message, ACK);
                sendMessage(socket, &message, 0);
            }
        }
        
    }
    fclose(fp);

}

// DONE
void execute_put(int socket){
    char *file_name;
    message_t message, response, putData;
    FILE *fp;
    struct stat st;
    int size = 0, flag, sequence,  i;

    file_name = malloc(STRING_MAX_SIZE * sizeof(char));
    scanf("%s", file_name);
    int fname_size = strlen(file_name);

    message = createMessage();
    response = createMessage();

    if (fname_size > MAX_DATA) {
        printf("Invalid file name: maximum allowed file name has length %d\n", MAX_DATA);
    }
    else {
        message.data_size = fname_size;
        message.sequence = 0;
        message.type = PUT;

        for(int i = 0; i < fname_size; i++) {
            message.data[i] = file_name[i];
        }

        verticalParity(&message);
    }

    // Manda o nome do arquivo
    while (response.type != OK) {
        sendMessage(socket, &message,  0);
        //printf("mandei put %s\n", file_name);
        // Espera um ok
        while (client_can_read() != 1);

        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }
    //printf("recebi ok\n");
    message = createMessage();
    setHeader(&message, ACK);
    sendMessage(socket, &message, 0);

    fp = fopen(file_name, "r");
    if (!fp) {
        printf("Não foi possível abrir %s.\n", file_name);
        return;
    }

    stat(file_name, &st); 
    size = st.st_size; // tamanho do arquivo

    message = createMessage();
    response = createMessage();

    message.data_size = sizeof(int);
    message.data[0] = size;
    message.type = PUT;
    verticalParity(&message);

    // Manda o tamanho do arquivo
    while (response.type != ACK) {
        sendMessage(socket, &message,  0);
        //printf("mandei tam put\n");
        // Espera um ok
        while (client_can_read() != 1);
        response = createMessage();
        if(recvMessage(socket, &response, 1) == 2 || recvMessage(socket, &response, 1) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
        // confere se houve erro
        if (response.type == ERRO){
            if (response.data[0] == NO_SPACE) {
                printf("Sem espaço no server para transferir o arquivo.\n");
            }
            return;
            
        }
    }

    // Manda um TX
    response = createMessage();
    while(response.type != ACK) {
        message = createMessage();
        setHeader(&message, TX);
        sendMessage(socket, &message, 1);
        //printf("Enviei o TX\n");

        // recebe ack do tx
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }
    // comeca a enviar os dados
    putData = createMessage();
    putData.sequence = sequence % MAX_SEQ;
    putData.data_size = MAX_DATA;
    setHeader(&putData, DADOS);
    flag = fread(putData.data, MAX_DATA, 1, fp);


    while (flag == 1) {
        verticalParity(&putData);
        sendMessage(socket, &putData, 1);
        // espera ack ou nack
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    
        // mandar a prox
        if(response.type == ACK) {
            //printf("recebi um ack\n");
            sequence++;
            putData = createMessage();
            putData.data_size = MAX_DATA;
            flag = fread(putData.data, MAX_DATA, 1, fp);
            putData.sequence = sequence % MAX_SEQ;
            setHeader(&putData, DADOS);

        }
    }
    
    for (i = 0; i < MAX_DATA; i++){
        if (putData.data[i] == 0) {
            break;
        }
    }
    
    printf("%d\n", i);
    response = createMessage();
    verticalParity(&putData);
    putData.data_size = i;
    // manda o final
    while (response.type != ACK) {
        sendMessage(socket, &putData, 1);
        while (server_can_read() != 1); 
        response = createMessage();
        if(recvMessage(socket, &response, 0) == 2 || recvMessage(socket, &response, 0) < 0){
            fprintf(stderr, "DON'T PANIC! \n EVERYTHING GONNA BE AL... \n");
            return;
        }
    }

    message = createMessage();
    setHeader(&message, FIM_TX);
    sendMessage(socket, &message, 1);
    
}