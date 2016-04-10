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
/*-------------------------------------------------------------------------------
CREAZIONE CLIENT CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./client -a 1500 127.0.0.1
PASSAGGI:
	CREAZIONE SOCKET
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/
char *prog_name;


int main(int argc, char *argv[]){

/*Apertura Connessione*/
/*NB--Stiamo usando e funzioni della libreria sockwrap.c che hanno già implementato il controllo degli errori,che quindi noi non scriveremo qui!*/

/*
uint32_t 		   taddr_n; //= 127.0.0.1;
*/
uint16_t		tport_n; //= 1500;
struct sockaddr_in	saddr;
int 			s;
int 			result;

prog_name = argv[0];

/*creazione della socket(famiglia,tipo,protocollo)*/
s = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

if (argc!=3)
		err_quit ("wrong number of parameters e.g. ./client 'address' 'port' ", prog_name);

		//salviamo in result ciò che ritorna la inet_aton (1 se ok, 0 se errore) e ci gestiamo l'errore
		result=inet_aton(argv[1],&(saddr.sin_addr));
			if ( result == 0 )
			{
			err_sys ("(%s) error - connect() failed", prog_name);
			}
		tport_n=atoi(argv[2]);       					//conversione_porta


/*inizializzazione di famiglia,porta e indirizzo_destinatario(saddr)*/
saddr.sin_family 	= AF_INET;
saddr.sin_port   	= htons(tport_n);
/*
saddr.sin_addr.s_addr	= atoi(saddr.sin_addr.s_addr);
saddr.sin_addr.s_addr	= htonl(saddr.sin_addr.s_addr);
*/

/*Connessione*/
Connect(s,(struct sockaddr*)&saddr,sizeof(saddr));

/*chiusura del socket*/
Close(s);

return 0;

}


/*-------------------------------------------------------------------------------*/
