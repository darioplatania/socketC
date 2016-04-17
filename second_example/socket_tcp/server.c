/*
***AUTORE: Dario Platania 'dariopl91@gmail.com', Marzo 2016
gcc -g -Wall -DTRACE -o server server.c errlib.c sockwrap.c
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
#include <sys/stat.h>
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

char * prog_name;

int main(int argc, char *argv[]) {

uint16_t		  tport_n; //= 1500;
struct        sockaddr_in	servaddr;
struct        sockaddr_in	clientaddr;
int 			    listenfd;
int           ac;
socklen_t     addrlen;
char          read[255];
char          sendbuff[1025];
char          file[50];
struct        stat sstr;
int           i=0;
int           j=0;
int           size1;
uint32_t      size,timestamp; //dimensione file
FILE          *fp;
int           byte_ricevuti;

prog_name = argv[0];

if(argc!=2)
  err_quit("wrong number of parameters e.g. ./server 'port'", prog_name);
  tport_n=atoi(argv[1]);

  /* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  trace (err_msg("(%s) socket created",prog_name));

  /* specify address to bind to */
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(tport_n);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientaddr.sin_family = AF_INET;

  /*BIND*/
  Bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  trace ( err_msg("(%s) listening on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

  /*LISTEN*/
  Listen(listenfd, 10); //LISTENQ NUMERO DI RICHIESTE PENDENTI


while(1) {
  trace( err_msg ("(%s) waiting for connections ...", prog_name) );
  addrlen = sizeof(struct sockaddr_in);

  ac = Accept(listenfd, (struct sockaddr*) &clientaddr, &addrlen);
  trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port)));


while(1){
byte_ricevuti=Recv(ac, read, 255, 0);
read[byte_ricevuti]='\0';
if (read[0]=='Q') break;
printf("File Richiesto: %s", read);
j=4;
for (i=0; i<byte_ricevuti; i++){
    if (read[j]=='\r') break;
    file[i]=read[j];
    j++;}
    file[i]='\0';
printf("Nome file effettivo: %s\n", file);
fp=fopen(file, "r");
if (fp==NULL) {
printf("Il file non esiste!\n");
Send(ac, "-ERR\r\n", 6, 0);
close(ac);
break;
}
/*FILE ESISTENTE--INVIO +OK CR LF*/
Send(ac, "+OK\r\n", 5, 0);

  /*DIMENSIONE FILE*/
  stat(file, &sstr);

  size1=((uint32_t)sstr.st_size);
  printf("Numero di byte su file: %u\n", size1);
  size=htonl(((uint32_t)sstr.st_size));
  Send(ac, &size, 4,0); //mando la size

  /*TIMESTAMP*/
  timestamp=((uint32_t)sstr.st_mtime);
  printf("Timestamp: %d\n", timestamp);
  timestamp=htonl(((uint32_t)sstr.st_mtime));
  Send(ac, &timestamp, 4,0);

  /*LEGGO IL FILE E LO MANDO A PEZZI DI 1024*/
  while (size1>0) {
    if (size1>=1024) {
      fread(sendbuff, 1, 1024, fp);
      Send(ac, sendbuff, 1024, 0);
      size1=size1-1024;
    }
    else {
      fread(sendbuff, 1, size1, fp);
      Send(ac, sendbuff, size1, 0);
      fclose(fp);
      size1=0;
   }
  }
}/*CHIUSURA SECONDO WHILE*/
}/*CHIUSURA PRIMO WHILE*/

}
