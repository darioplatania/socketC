/*
***AUTORE: Dario Platania 'dariopl91@gmail.com', Marzo 2016
gcc -g -Wall -DTRACE -o client client.c errlib.c sockwrap.c
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

#define SIZE_MAX_BUFF 255
/*-------------------------------------------------------------------------------
CREAZIONE CLIENT CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./client -a 1500 127.0.0.1
PASSAGGI:
	CREAZIONE SOCKET
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/
char *prog_name;


int main(int argc, char *argv[]){

/*Apertura Connessione*/

/*
uint32_t 		   taddr_n; //= 127.0.0.1;
*/
uint16_t		tport_n; //= 1500;
struct      sockaddr_in	saddr;
int 			  listenfd;
int 			  result;
int 				numero;
char				nomefile[30];
char 				dest[36];
char				cr[] = "\r";
char				lf[] = "\n";
int         len;
//char 				server_reply[SIZE_MAX_BUFF];
char        date[SIZE_MAX_BUFF];
FILE        *fp;

/*
char 				*msg;
int 				len;
*/

prog_name = argv[0];

/*creazione della socket(famiglia,tipo,protocollo)*/
listenfd = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

if (argc!=3)
		err_quit ("wrong number of parameters e.g. ./client 'address' 'port' ", prog_name);

		//salviamo in result ciò che ritorna la inet_aton (1 se ok, 0 se errore) e ci gestiamo l'errore
		result=inet_aton(argv[1],&(saddr.sin_addr));
			if ( result == 0 )
			{
			err_sys ("(%s) error - connect() failed", prog_name);
			}
		tport_n=atoi(argv[2]); //conversione_porta


/*inizializzazione di famiglia,porta e indirizzo_destinatario(saddr)*/
saddr.sin_family 	= AF_INET;
saddr.sin_port   	= htons(tport_n);
/*
saddr.sin_addr.s_addr	= atoi(saddr.sin_addr.s_addr);
saddr.sin_addr.s_addr	= htonl(saddr.sin_addr.s_addr);
*/

/*Connessione*/
Connect(listenfd,(struct sockaddr*)&saddr,sizeof(saddr));

while(1)
	{

	printf("Digita un numero:\n[1] Richiedi File\n[0] Quit\n");
	scanf("%d", &numero);

	switch (numero) {

  case 0:
		   printf("Bye!\n");
			 Close(listenfd);
			 exit(1);
		   break;

	case 1:
	   printf("Inserisci nome file: ");
		 scanf("%s",nomefile);
		 *dest = '\0';
		 sprintf(dest,"%s","GET ");
		 strcat(dest,nomefile);
		 strcat(dest,cr);
		 strcat(dest,lf);
		 //printf("Dopo la concatenazione: %s\n",dest);  //GET nomefile.txt
		 len = strlen(dest);

		 /*INVIO IL NOME DEL FILE RICHIESTO AL SERVER*/
		 Send(listenfd,dest,len,0);
		 printf("File richiesto: %s",dest);

		 /*RICEVO IL CONTENUTO DEL FILE*/

	   break;

	default:
	   printf("Scelta non corretta!\n");
	   break;
}

  /*RICEVO DAL SERVER*/

	/*
	Recv(listenfd,server_reply,SIZE_MAX_BUFF,0);
	printf("Ricezione completa: %s\n",server_reply);
	*/

	int byte_ricevuti = Recv(listenfd,date,SIZE_MAX_BUFF,0);
	date[byte_ricevuti] = '\0';
	printf("Ricevuto e copiato: %s\n",date);

	/*METTO IN PROVA.TXT IL CONTENUTO DEL FILE RICEVUTO DAL SERVER*/
	fp = fopen( nomefile , "w" );
  fwrite(date , 1 ,byte_ricevuti , fp );

	/* chiude il file */
	fclose(fp);

}

/*chiusura del socket*/
Close(listenfd);

return 0;

}


/*-------------------------------------------------------------------------------*/
