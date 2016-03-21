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
CREAZIONE CLIENT UDP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./client -a 1500 127.0.0.1 datagram
PASSAGGI:
	CREAZIONE SOCKET
	INVIO DATAGRAM PRESO DA TERMINALE
  ATTENDE RECV DAL SERVER E SE CE L'HA RITORNA COSA HA SCRITTO IL SERVER
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/

char *prog_name;

int main(int argc, char *argv[]) {

uint16_t		tport_n; //= 1500;
struct      sockaddr_in	saddr;
char        *msg;
int 			  s;
int         result;
int         len;
int         recv_size;
char        datagram[SIZE_MAX_BUFF];


  prog_name = argv[0];

  /*creazione della socket(famiglia,tipo,protocollo)*/
  s = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  trace ( err_msg("(%s) socket created",prog_name) );

  /*controllo immissione e.g. ./client_udp 'indirizzo' 'porta' 'datagram'*/
  if (argc!=4)
  		err_quit ("wrong number of parameters e.g. ./client 'address' 'port' 'datagram' ", prog_name);
  		//salviamo in result ciò che ritorna la inet_aton (1 se ok, 0 se errore) e ci gestiamo l'errore
  		result=inet_aton(argv[1],&(saddr.sin_addr));
  			if ( result == 0 )
  			{
  			err_sys ("(%s) error - connect() failed", prog_name);
  			}
  		tport_n=atoi(argv[2]); 	//conversione_porta
      msg = argv[3];
      len = strlen(argv[3]);  //prendo la lunghezza di datagram e la controllo perchè non deve superare 31
      if( len > 31 )
      {
        err_sys ("(%s) datagram maggiore di 31 caratteri", prog_name);
      }

/*inizializzazione di famiglia,porta e indirizzo_destinatario(saddr)*/
saddr.sin_family 	= AF_INET;
saddr.sin_port   	= htons(tport_n);
trace ( err_msg("(%s) destination is %s:%u", prog_name, inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port)) );

    /*send_to verso il server*/
    Sendto (s,msg,len,0,(struct sockaddr*)&saddr,sizeof(saddr));
    trace ( err_msg("(%s) data has been sent: '%s'", prog_name,msg) );

    /*dopo 5 sec chiude se ci sono problemi */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    /*recv_from dal server*/
    socklen_t fromlen = sizeof(saddr);
    recv_size = Recvfrom (s,datagram,SIZE_MAX_BUFF,0,(struct sockaddr*)&saddr,&fromlen);
    datagram[recv_size] = '\0';
    trace ( err_msg("(%s) receive string: '%s'",prog_name,datagram) );

  return 0;
}
