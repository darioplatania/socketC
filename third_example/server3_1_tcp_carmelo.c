/*
***AUTORE: Carmelo Riolo Marzo 2016
gcc -g -Wall -DTRACE -o server3_1_tcp server3_1_tcp.c errlib.c sockwrap.c
*/

/* Headers */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
/* used for timestamp handling */
#include <sys/stat.h>
#include <time.h>

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>

#include "errlib.h"
#include "sockwrap.h"

#define LISTENQ 15
#define BUFSIZE 255
#define BLOCK_SIZE 4096

char *prog_name;

/* Contatore per gestire al massimo 3 processi figli */
int pid_counter = 0;
int pid_array[3];

/* Signal Handler */
void sighandler()
{

   while(waitpid(-1,NULL,WNOHANG)>0)
   {
	//res=waitpid(pid_array[i],NULL,WNOHANG);
	 //if(res>0)
	pid_counter--;
   	printf("[SA]pid_counter: %d\n",pid_counter);
   }





}


int main(int argc, char* argv[]){

    /* Socket variables */

    int s;
    int connected_socket;
    uint16_t port;
    struct sockaddr_in saddr;
    uint32_t saddr_len;
    ssize_t n_bytes;

    char buf[BUFSIZE];

    /* File variables */
    FILE *file;
    char nome_file[BUFSIZE];

    char content[BLOCK_SIZE];


    char success_response_head[6];
    strcpy(success_response_head,"+OK\r\n");
    success_response_head[5]='\0';

    char error_response[7];
    strcpy(error_response,"-ERR\r\n");
    error_response[6]='\0';
    //uint32_t read_bytes;

    uint32_t file_size;
    time_t last_mod;

    int i;

    /* Process Identifier */
    int pid;
    int status;


    /* defined in sys/stat */
    /* le informazioni sul timestamp verranno scritte in tale struct dalla funzione fstat */
    struct stat timestamp;

    /* sigset for sigprocmask to block SIGCHLD and avoid race condition */
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set,SIGCHLD);


    if(argc!=2)
    {
        printf("%s <port>\n",prog_name);
        exit(-1);
    }

    /* Set Signal Handler */
    signal(SIGCHLD, sighandler);

    port=atoi(argv[1]);
    printf("Init Port:\t%hu\n",port);

    /* Creo Socket */
    s = Socket(AF_INET,SOCK_STREAM,0);

    /* Bind */
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    saddr.sin_addr.s_addr=htonl(INADDR_ANY);

    Bind(s,(struct sockaddr *)&saddr,sizeof(saddr));

    /* Listen */

    Listen(s,LISTENQ);



    printf("Server listening on %s:%hu\n",inet_ntoa(saddr.sin_addr),ntohs(saddr.sin_port));





    /* Accept */

    while(1){

        saddr_len = sizeof(saddr);


	/* connection handling */

	while(1){

//		sigprocmask(SIG_BLOCK,&signal_set,NULL);

		if(pid_counter<3){

			/* Creating new connected socket */
			connected_socket = Accept(s,(struct sockaddr*)&saddr,&saddr_len);

			/* Fork() */
			pid = fork();



			if(pid>0){

				/* Processo padre */
				/* pid - Process Identifier Figlio */
				pid_array[pid_counter]=pid;
				pid_counter++;

				Close(connected_socket);


			}else if(pid==0){

				/* Processo figlio */
				printf("Forked new process..\n");

				/* chiudo socket in ascolto */
				Close(s);
				break;

			}else
				printf("Non è stato possibile creare il processo figlio\n");
		} else {
			printf("Massimo numero di processi raggiunto\n");

			wait(&status);
			pid_counter--;
			printf("Killed by Father - %d\n",pid_counter);
		}


//		sigprocmask(SIG_UNBLOCK,&signal_set,NULL);
        }

	/* end connection handling */

        printf("Received connection from: %s:%hu\n",inet_ntoa(saddr.sin_addr),ntohs(saddr.sin_port));
        printf("Wating for input\n");


        do
        {

           n_bytes = recv(connected_socket,buf,sizeof(buf),0);
           buf[n_bytes]='\0';


            if(n_bytes!=-1 && strncmp(buf,"QUIT\r\n",6))
            {

                strncpy(nome_file,buf+4,n_bytes-6);
                nome_file[n_bytes-6]='\0';


                file = fopen(nome_file,"r");
                if(file)
                {
                    printf("File Founded: %s\n",nome_file);
                    if(stat(nome_file,&timestamp)>=0)
                    {


                        file_size=htonl(timestamp.st_size);
                        last_mod=htonl(timestamp.st_mtime);



                        for(i=0;i<4;i++)


                            switch(i){

                                case 0:
                                    /* Invio +OK\r\n */
                                    Send(connected_socket,success_response_head,1,0);
                                    Send(connected_socket,success_response_head+1,strlen(success_response_head)-1,MSG_NOSIGNAL);


                                    break;
                                case 1:
                                    /* Invio file size in Network Byte Order */
                                    Send(connected_socket,&file_size,sizeof(uint32_t),0);


                                    break;
                                case 2:
                                    /* Invio Timestamp in Network Byte Order */
                                    Send(connected_socket,&last_mod,sizeof(uint32_t),0);


                                    break;
                                case 3:

                 		    /* Invio File Content */
                                    if(timestamp.st_size<BLOCK_SIZE){
					n_bytes = fread(content,sizeof(char),timestamp.st_size,file);
					content[n_bytes]='\0';
					writen(connected_socket,content,n_bytes);
				    }else{

					    for(i=0;i<timestamp.st_size;)
					    {
						n_bytes = fread(content,sizeof(char),BLOCK_SIZE,file);
						content[n_bytes]='\0';
						writen(connected_socket,content,n_bytes);
						if((i+BLOCK_SIZE)>=timestamp.st_size){

							n_bytes = fread(content,sizeof(char),timestamp.st_size-i,file);
							content[n_bytes]='\0';
							writen(connected_socket,content,n_bytes);

						}

						i+=BLOCK_SIZE;
					    }
				    }



                           	    fclose(file);
                                    printf("File trasferito\n");





                                    break;




                            }   /* Fine Switch */





                    }
                }
                else
                {
                    /* Invio -ERR\r\n */
                    printf("File inesistente\n");
                    Send(connected_socket,error_response,1,0);
                    Send(connected_socket,error_response+1,strlen(error_response)-1,0);


                 }
             }


        }while(strncmp(buf,"QUIT\r\n",6));

	Close(connected_socket);

    	break;


    }

    /* Se padre sta terminando */
    if(pid>0)
	Close(s);


    return 1;

}
