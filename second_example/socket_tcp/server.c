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
#include <sys/stat.h>
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
#define LISTENQ 15

/*-------------------------------------------------------------------------------
CREAZIONE SERVER UDP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./server 1500
PASSAGGI:
	CREAZIONE SOCKET
  BIND
  LISTEN
  ACCEPT
	RICEZIONE  DAL CLIENT
  INVIO AL CLIENT DEL DATO RICEVUTO
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/


char *prog_name;

int main(int argc, char *argv[]) {

uint16_t		tport_n; //= 1500;
struct      sockaddr_in	servaddr;
struct      sockaddr_in	clientaddr;
int 			  listenfd;
int         ac;
//int         err=0;
socklen_t   addrlen;
char        date[SIZE_MAX_BUFF+1];
char        file[SIZE_MAX_BUFF+1];
int         i,j;
FILE        *fp;
char        buf[200];
char        *res;
char 				dest[SIZE_MAX_BUFF];
char 				err_var[SIZE_MAX_BUFF];
char				first_char[] = "+OK";
char				cr[] = "\r";
char				lf[] = "\n";
//socklen_t   cliaddrlen = sizeof(clientaddr);
//socklen_t   clientaddr_len;
//int         recv_size;


prog_name = argv[0];

/*controllo immissione */

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
  Listen(listenfd, LISTENQ); //LISTENQ NUMERO DI RICHIESTE PENDENTI

  while(1) {
    trace( err_msg ("(%s) waiting for connections ...", prog_name) );
    addrlen = sizeof(struct sockaddr_in);

    /*ACCEPT*/
    ac = Accept(listenfd, (struct sockaddr*) &clientaddr, &addrlen);
    trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port)));

    /*RICHIESTA FILE DAL CLIENT*/
    *date = '\0';
    *file = '\0';
    Recv(ac,date,SIZE_MAX_BUFF,0);
    trace ( err_msg("(%s) File Richiesto: %s",prog_name,date));

    /*MI ESTRAGGO IL NOME DEL FILE E VEDO SE ESISTE O MENO E LO DICO AL CLIENT*/
    j = 0;
    for (i=4; i<SIZE_MAX_BUFF && date[i]!= '\r'; i++)
    {
      file[j]=date[i];
      j++;
    }
      file[j]='\0';
      /* alla fine salvo in file l'effettivo nome cercato,ripulito dalla parte +OK ecc-- */
      trace( err_msg ("(%s) Nome file effettivo: %s", prog_name,file) );

     /*APRE IL FILE*/
    fp=fopen(file, "r");
      if( fp==NULL )
        {
            /* se non esiste il file richiesto, il server risponde con il messaggio di errore */
            sprintf(err_var,"%s","-ERR");
            strcat(err_var,cr);                         /*concateno cr*/
            strcat(err_var,lf);                        /*concateno lf*/
            trace( err_msg ("(%s)%s", prog_name,err_var) );
            Send(ac,err_var,sizeof(err_var),0);
        }
      else
        {
            trace( err_msg ("(%s) File Esistente", prog_name) );

            /* legge e stampa ogni riga */
            while(1)
            {
             res=fgets(buf, 200, fp);
             if( res==NULL )
               break;
              trace( err_msg ("(%s) Il File contiene: %s", prog_name,buf) );

              /*PRENDO IL FILE CHE MI ARRIVA E VEDO LA SUA DIMENSIONE*/
              struct stat st;
              stat(file, &st);
              int size = st.st_size;
              printf("Dimensione in byte: %d\n", size);

              /*PRENDO IL FILE CHE MI ARRIVA E VEDO IL SUO TIMESTAMP*/
              struct stat attr;
              stat(file, &attr);
              printf("Last modified time: %s", ctime(&attr.st_mtime));

             /*---prendo il contenuto del file e gli incateno +OK CR LF Byte Timestamp Filename---*/
             //*dest = '\0';
             sprintf(dest,"%s" "%s" "%s" "%d" "%s" "%s",first_char,cr,lf,size,ctime(&attr.st_mtime),buf);
             //sprintf(dest,"%s","+OK");
             //strcat(dest,first_char);                  /*concateno +OK*/
        		 //strcat(dest,cr);                         /*concateno cr*/
        		 //strcat(dest,lf);                        /*concateno lf*/
             //strcat(dest,size);                     /*concateno byte_ (attenzione si deve fare il cast!)*/
             //strcat(dest,timestamp_sec)
             //strcat(dest,buf);                    /*concateno contenuto del file*/
             trace( err_msg ("(%s) Il File da inviare è: %s", prog_name,dest) ); /*IN dest ORA SI TROVA +OK CR LF Byte Timestamp Filename*/
            }

            /* chiude il file */
            fclose(fp);

            /* SEND VERSO IL CLIENT DI TUTTO IL CONTENUTO DI DEST --> +OK CR LF Byte Timestamp Filename */
            /* per ora invio solo il contenuto del file senza tutto dest */

            /*
            Send(ac,dest,strlen(dest),0);
            trace( err_msg ("(%s) Invio... %s", prog_name,dest) );
            */

            /* SEND VERSO IL CLIENT CON IL CONTENUTO DI BUFF --> Filename */
            Send(ac,buf,strlen(buf),0);
            trace( err_msg ("(%s) Invio contenuto... %s", prog_name,buf) );

        }/*chiusura else*/




    /*CLOSE*/
		//trace( err_msg ("(%s) - connection closed by %s", prog_name, (err==0)?"client":"server") );

  }/*chiusura while*/

  return 0;
}
