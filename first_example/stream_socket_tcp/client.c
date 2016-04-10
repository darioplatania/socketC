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

#define SIZE_MAX_STRING 2
#define SIZE_MAX_BUF 255
/*-------------------------------------------------------------------------------
CREAZIONE CLIENT TCP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./client -a 1500 127.0.0.1
PASSAGGI:
	CREAZIONE SOCKET
	INVIO DATI: (DUE NUMERI)
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
struct      sockaddr_in	saddr;
int 			  s;
int 			  result;
char        msg[SIZE_MAX_STRING];
char				buf[SIZE_MAX_BUF];
int         len;
int 				res,op1,op2;
int 				nconv;

prog_name = argv[0];

/*creazione della socket(famiglia,tipo,protocollo)*/
s = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
trace ( err_msg("(%s) socket created",prog_name) );

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
trace ( err_msg("(%s) destination is %s:%u", prog_name, inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port)) );

/*prendo dati da input*/
printf("Inserisci primo numero da inviare: ");
	scanf("%d", &op1);
	printf("Inserisci secondo numero da inviare: ");
	scanf("%d", &op2);


/*
	while( (op1 != '\r') || (op2 != '\n') ) {
		printf("Inserisci 2 numeri da inviare: ");
		scanf("%d %d",&op1,&op2);
	}
*/
	/*copia la stringa in msg*/
	sprintf(msg, "%d %d\n", op1, op2);

	/*invio della stringa*/
	len = strlen(msg);
	Write(s,msg,len);
	trace ( err_msg("(%s) - dato inviato: '%s'", prog_name,msg) );

	/*ricevo risultato dal server e vedo se è un numero o testo*/
	Readline(s, buf, SIZE_MAX_BUF);

	nconv = sscanf(buf, "%d", &res);
	/*se è 1 vuol dire che la sscanf è andata a buon fine perchè è un numero*/
	if (nconv==1) {
		trace ( err_msg("(%s) - Somma Ricevuta dal server: '%d'", prog_name,res) );
	} else {
		trace ( err_msg("(%s) - Messaggio: '%s'", prog_name,buf) );
	}

/*chiusura del socket*/
Close(s);
trace ( err_msg("(%s) --Socket chiusa--", prog_name) );

return 0;

}


/*-------------------------------------------------------------------------------*/
