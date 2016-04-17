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
char * prog_name;

int main(int argc, char *argv[]) {
	uint16_t		  tport_n; //= 1500;
	uint32_t 			size;
  prog_name = 	argv[0];
  int 					listenfd;
	int 			    result;
  char 					buffer[500];
  char 					file[20];
	struct        sockaddr_in	saddr;


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

	/*Connessione*/
	Connect(listenfd,(struct sockaddr*)&saddr,sizeof(saddr));


  while(1) {
	  sprintf(buffer, "%s", "GET");
	  strcat(buffer, " ");
	  printf("Richiedi File: ");
	  scanf("%s", file);
	  strcat(buffer, file);
	  strcat(buffer, "\r\n");
	  Send(listenfd, buffer, strlen(buffer),0);
	  int k=Recv(listenfd, buffer, 5,0);
	  if (k==0) {
	    printf("Connection Closed by Server");
	    break;
	  }
	  buffer[5]='\0';
	  printf("%s", buffer);
	  if (buffer[0]=='-') {
	    printf("File inesistente\n");
	    printf("%s\n", buffer);
	    close(listenfd);
	    break;
	  }
	  readn(listenfd, buffer, 4);
	  size=ntohl(*((uint32_t*)buffer));
	  printf("Dimensione in byte ricevuti dal server: %u\n", size);
	  Recv(listenfd, buffer, 4,0);
	  printf("Timestamp ricevuto dal server: %u\n", ntohl(*((uint32_t*)buffer)));
	  char lettura[1025];
	  FILE *fp;
	  fp=fopen(file, "wb");
	  while (size>0) {
	    if (size>=1024) {
	      Recv(listenfd,lettura,1024,0);
	      size=size-1024;
	      fwrite(lettura, 1, 1024, fp);
	    }
			else {
		    Recv(listenfd,lettura,size,0);
		    lettura[size]='\0';
		    fwrite(lettura, 1, size, fp);
		    fclose(fp);
		    size=0;
	  }
	}/*CHIUSURA PRIMA WHILE*/
}/*CHIUSURA SECONDO WHILE*/
close(listenfd);

return 0;
}
