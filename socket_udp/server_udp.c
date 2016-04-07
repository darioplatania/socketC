/*
***AUTORE: Dario Platania 'dariopl91@gmail.com', Marzo 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errlib.h"
#include "sockwrap.h"

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

#define SIZE_MAX_BUFF 255

/*-------------------------------------------------------------------------------
CREAZIONE SERVER UDP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./server 1500
PASSAGGI:
	CREAZIONE SOCKET
	RICEZIONE DATAGRAM DAL CLIENT
  INVIO AL CLIENT DEL DATAGRAM RICEVUTO
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/

char *prog_name;

int main(int argc, char *argv[]) {

uint16_t		tport_n; //= 1500;
struct      sockaddr_in	servaddr;
struct      sockaddr_in	saddr;
int 			  listenfd;
char        datagram[SIZE_MAX_BUFF+1];
socklen_t   saddr_len;
int         recv_size;


  prog_name = argv[0];

  /*controllo immissione e.g. ./server_udp 'indirizzo' */

  if (argc!=2)
    err_quit ("wrong number of parameters e.g. ./server 'address'", prog_name);
    tport_n=atoi(argv[1]);

  /*creazione della socket(famiglia,tipo,protocollo)*/
  listenfd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  trace ( err_msg("(%s) socket created",prog_name) );

  /* specify address to bind to */
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(tport_n);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_family = AF_INET;

  /*bind*/
  Bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	trace ( err_msg("(%s) listening for UDP packet %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

  /*il server resta sempre in attesa di ricevere un pacchetto senza mai chiudersi*/

  while (1) {
		trace( err_msg ("(%s) waiting for a packet ...", prog_name) );

    /*recv_from dal client*/
    saddr_len = sizeof(saddr);
    recv_size = Recvfrom (listenfd,datagram,SIZE_MAX_BUFF,0,(struct sockaddr*)&saddr,&saddr_len);
    datagram[recv_size] = '\0';
    trace ( err_msg("(%s) receive string: '%s'",prog_name,datagram) );

    /*send_to verso il client*/
    Sendto(listenfd, datagram, strlen(datagram), 0,(struct sockaddr *)&saddr, saddr_len);
    trace ( err_msg("(%s) data has been sent: '%s'", prog_name,datagram) );

  }

  return 0;
}
