/*
***AUTORE: Dario Platania 'dariopl91@gmail.com', Marzo 2016
gcc -g -Wall -DTRACE -o server_3_1 server_3_1.c errlib.c sockwrap.c
*/

/*
*** SERVER TCP LAB 3.1 CON GESTIONE 3 CLIENT E TIMEOUT (APPELLO GIUGNO)***
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
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

int pid_array[3];
int n_child = 0;

/* Signal Handler */
void sighandler() {
   while(waitpid(-1, NULL, WNOHANG) > 0) {
	    n_child--;
   	  printf("[SA]pid_counter: %d\n",n_child);
   }
}

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
  int           childpid;
  int           status;
  sigset_t      signal_set;

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  sigemptyset(&signal_set);
  sigaddset(&signal_set,SIGCHLD);

  prog_name = argv[0];

  if(argc!=2)
    err_quit("wrong number of parameters e.g. ./server 'port'", prog_name);
  tport_n=atoi(argv[1]);
  signal(SIGCHLD, sighandler);
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

    if(n_child < 3) {

    ac = Accept(listenfd, (struct sockaddr*) &clientaddr, &addrlen);
    trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port)));

    /*FACCIO LA FORK*/
    if((childpid=fork())<0)
      err_msg("fork() failed");
    else if (childpid > 0) {
      /* processo padre */
      pid_array[n_child] = childpid;
      n_child++;
      close(ac); /* chiudo socket */
    } else {
      /* processo figlio */
      close(listenfd); /* chiudo socket del padre */
      while(1) {
        setsockopt (ac, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        byte_ricevuti=Recv(ac, read, 255, 0);
        read[byte_ricevuti]='\0';
        /*Se ricevo Q esce*/
        if (strncmp(read, "QUIT", (size_t)4) == 0)
          break;
        /*Se ricevo ctrl-c esce*/
        if(*read == '\0') {
          trace( err_msg ("(%s) - connection closed by client", prog_name) );
          trace( err_msg ("(%s) waiting for connections ...", prog_name) );/*se chiudo con ctrl-c questa trace mi stampa il waiting prima di fare il break e tornare  sull'accept*/
          break;
        }
        printf("File Richiesto: %s", read);
        j=4;
        for (i=0; i<byte_ricevuti; i++){
          if (read[j]=='\r')
            break;
          file[i]=read[j];
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
        Send(ac, &timestamp, 4, 0);

        /*LEGGO IL FILE E LO MANDO A PEZZI DI 1024*/
        while (size1>0) {
          if (size1>=1024) {
            fread(sendbuff, 1, 1024, fp);
            Send(ac, sendbuff, 1024, 0);
            size1=size1-1024;
          } else {
            fread(sendbuff, 1, size1, fp);
            Send(ac, sendbuff, size1, 0);
            fclose(fp);
            size1=0;
          }
        }
        printf("stampa\n\n");
      }/*CHIUSURA SECONDO WHILE*/
      exit(0);
      }/*CHIUSURA PROCESSO FIGLIO*/
    } else {
      // Processo padre quando ci sono già tre processi figli attivi
      printf("Raggiunto il numero massimo di processi figli ammessi\n");
      wait(&status);
      n_child--;
      printf("Processo %d terminato\n", n_child);
    }
  }/*CHIUSURA WHILE PRINCIPALE*/
}
