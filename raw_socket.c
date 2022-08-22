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

int SERVER_READ = 2;
int CLIENT_READ = 2;

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

void sendMessage(int socket_id, message_t* message, message_t* response, int sender) {
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
    return;
  }
  else
    recvMessage(socket_id, message, wait_for); 
}
