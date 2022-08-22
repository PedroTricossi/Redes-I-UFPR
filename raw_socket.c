#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include<netinet/ip_icmp.h>	//Provides declarations for icmp header
#include<netinet/udp.h>	//Provides declarations for udp header
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header
#include<netinet/if_ether.h>	//For ETH_P_ALL
#include<sys/time.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>

#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>	//For standard things
#include<stdlib.h>	//malloc
#include<string.h>	//strlen

#include "kermit.h"

int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

void sendResponse(int socket_id, message_t* response) {
  if(send(socket_id, response, sizeof(*response), 0) == -1) {
    perror("Send failed");
  }
}

int recvResponse(int socket_id, message_t* message, message_t* response) {
  unsigned char response_type;
  int count = 0; // Eliminates duplicated causedby loopback + 1 reading its own message
  int timeout = 5;
  time_t start_t, end_t;
  double diff_t;
  time(&start_t);

  while ((diff_t < timeout) && (count < 3)) {
    if(recv(socket_id, response, sizeof(*response), 0) == -1) {
      perror("Received failed");
    }
    else {
        if(response->marker == MARKER) {
          count++;
          // First catch must be its own message
          if((count == 1) && (response->type != message->type)) count--;
        }
    }
    // Increments timer
    time(&end_t);
    diff_t = difftime(end_t, start_t);
  }

  changePermission('y');
  if(response->marker == MARKER) {
    response_type = response->type;
    // Returns zero on worng parity and on NACK
    if(response_type == NACK_T) {
      return 0;
    }
    else if((response_type != ACK_T) && (!checkParity(response))) {
      return 0;
    }
  }
  else { // Timeout
    errorHeader(response, 5);
    return 0;
  }

  // Return 1 if got response
  return 1;
}

void sendMessage(int socket_id, message_t* message, message_t* response, int sender) {
  message->sender = sender;
  fprintf(stdout, "escreveu");
  if(write(socket_id, message, sizeof(*message)) == -1) {
    perror("Send failed");
  }

  // organiza file descriptor para timeout
  struct pollfd fd;
  fd.fd = socket_id;
  fd.events = POLLIN;

  if( poll(&fd, 1, 1) )
    read(socket_id, message, sizeof(*message));
}

void recvMessage(int socket_id, message_t* message, int wait_for) {
  int count = 0; // Eliminates duplicate message caused by loopback


  struct pollfd fd;
  fd.fd = socket_id;
  fd.events = POLLIN;

  // int retorno = poll(&fd, 1, 5*1000);
  // if( retorno == 0 )
  //   return;
  // else if( retorno < 0 )
  //   return;

  while(count < 1) {
    if(read(socket_id, message, sizeof(*message)) == -1) {
      perror("Received failed");
    }
    else {
      if(message->marker == MARKER) {
        count++;
      }
    }
  }

  if (message->sender != wait_for)
    recvMessage(socket_id, message, wait_for);
  
  else
    message->sender = 8;
}
