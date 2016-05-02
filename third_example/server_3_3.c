/*
***AUTORE: Dario Platania 'dariopl91@gmail.com', Marzo 2016
gcc -g -Wall -DTRACE -o server_3_3 server_3_3.c errlib.c sockwrap.c
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
#include <sys/wait.h>

#include "errlib.h"
#include "sockwrap.h"

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char * prog_name;

uint16_t		  tport_n; //= 1500;
struct        sockaddr_in	servaddr;
struct        sockaddr_in	clientaddr;
int 			    listenfd;
int           ac;
socklen_t     addrlen;
char          lettura[255];
char          sendbuff[1025];
char          file[50];
struct        stat sstr;
struct        timeval tval;
fd_set        cset;
int           i=0;
int           j=0;
int           size1;
uint32_t      size,timestamp; //dimensione file
FILE          *fp;
int           byte_ricevuti;
int           i;
int           nchildren = 0;
int           *childpid;
char          msg[200];
int           n;
int           status;
int i;

void sighandler(){
  for(i = 0; i < nchildren; i++){
    kill(childpid[i],SIGTERM);
  }
  while (wait(&status)>0);
    exit(0);
}


int main(int argc, char **argv)
{

  prog_name = argv[0];

  if(argc!=3)
    err_quit("wrong number of parameters e.g. ./server 'port' 'child_number'", prog_name);
    tport_n=atoi(argv[1]);
    nchildren = atoi(argv[2]);
    if(nchildren>10){
      err_quit("You can not create more than 10 child process", prog_name);
    }


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

    childpid=calloc(nchildren,sizeof(pid_t)); /*mi alloco lo spazio necessario*/
    for(i = 0; i < nchildren; i++){

      if((childpid[i]=fork())<0)
      err_msg("fork() failed");

        else if (childpid[i] > 0)
        {
        /* processo padre */
        }

        else {
        /* processo figlio */
            while(1) {
            trace( err_msg ("(%s) waiting for connections ...", prog_name) );
            addrlen = sizeof(struct sockaddr_in);
            ac = Accept(listenfd, (struct sockaddr*) &clientaddr, &addrlen);
            trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port)));

              while(1){
                /*inizio struttura select*/
                FD_ZERO(&cset); FD_SET(ac,&cset);
                int t = 120;
                tval.tv_sec = t;
                tval.tv_usec = 0;
                n = Select(FD_SETSIZE,&cset,NULL,NULL,&tval);
                /*fine struttura select*/

                if(n>0){
                byte_ricevuti=Recv(ac, lettura, 255, 0);
                lettura[byte_ricevuti]='\0';
                /*Se ricevo Q esce*/
                if (lettura[0]=='Q') break;
                /*Se ricevo ctrl-c esce*/
                if(*lettura == '\0'){
                  trace( err_msg ("(%s) - connection closed by client", prog_name) );
                  break;
                }
                printf("File Richiesto: %s", lettura);
                j=4;
                for (i=0; i<byte_ricevuti; i++){
                    if (lettura[j]=='\r') break;
                    file[i]=lettura[j];
                    j++;
                  }
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
                }/*CHIUSURA IF RECV */
              else{
                close(ac);
                printf("No response after %d second\n",t );
                break;
              }/*CHIUSURA ELSE NO_RESPONSE*/
            }/*CHIUSURA WHILE RECV*/
          }/*CHIUSURA WHILE CONNECTION*/
        }/*CHIUSURA ELSE*/
    }/*CHIUSURA FOR*/
    while(1){
      signal(SIGINT,sighandler);
    }
return 0;
}
