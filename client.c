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

int main() {
    char *socket_mode = "lo";
    int socket = 0;

    socket = ConexaoRawSocket(socket_mode);
    
    char cwd[100];
    char comando[8] = "";

    int seq = 0;

    createFile();

    while(1) { 
        printCurrentDir();
        scanf("%s", comando);

        if (strncmp("lcd", comando, 3) == 0){
            execute_cd_local();
        }  
        else if(strncmp("cd", comando, 2) == 0){
            execute_cd_server(socket);
        } 
        else if(strncmp("lmkdir", comando, 6) == 0){
            execute_mkdir_local();
        } 
        else if(strncmp("mkdir", comando, 5) == 0){
            execute_mkdir_server(socket);
        } 
        else if (strncmp("lls", comando, 3) == 0){
            execute_ls_local();
        }
        else if(strncmp("ls", comando, 6) == 0){
            execute_ls_server(socket);
        } 
        else if (strncmp("get", comando, 3) == 0){
            execute_get(socket);
        } else if (strncmp("put", comando, 3) == 0){
            execute_put(socket);
        }
    }

    return 0;
}