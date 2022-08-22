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
#include "server_lib.h"
#include "local_lib.h"

int main() {
    char *socket_mode = "lo";
    message_t package;
    int seq = 0;
    int socket = 0;
    unsigned char command;

    socket = ConexaoRawSocket(socket_mode);

    while(1){


        if(server_can_read() == 1) {
            printf("Server reading\n");
            package = createMessage();
            recvMessage(socket, &package, 0);
        } else {
            command = 16; // Impossible command
        }


        switch (package.type)
        {
        case 7:
            execute_cd(&package, socket);
            break;
        
        case 1:
            execute_ls(&package, &socket);
            break;
        
        case 2:
            execute_mkdir(&package, &socket);
            break;
        
        case 3:
            execute_get_server();
            break;

        case 4:
            execute_put_server();
            break;
        
        default:
            break;
        }
    }

    return 0;
}