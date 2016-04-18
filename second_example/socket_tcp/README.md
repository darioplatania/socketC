# Client and Server TCP stream

#SERVER STEP
```
/*-------------------------------------------------------------------------------
CREAZIONE SERVER TCP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./server 1500
PASSAGGI:
	CREAZIONE SOCKET
  BIND
  LISTEN
  ACCEPT
  RICHIESTA FILE DAL CLIENT
  INVIO CONTENUTO AL CLIENT DEL FILE
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/
```

#CLIENT STEP
```
/*-------------------------------------------------------------------------------
CREAZIONE SERVER TCP CHE PRENDE DA TERMINALE I PARAMENTRI e.g. ./server 1500
PASSAGGI:
	CREAZIONE SOCKET
  RICHIESTA DI UN FILE AL SERVER
  RICEZIONE CONTENUTO DEL FILE DAL SERVER
  MEMORIZZAZIONE DEL FILE
	CHIUSURA SOCKET
-------------------------------------------------------------------------------*/
```

# Ready

## To clone project:

```
mkdir /path/to/your/project

git clone git@bitbucket.org:darioplatania/socketC.git
```
### open two terminal and type

###Server executable => open terminal go to the project folder socket_tcp/ClientFolder and type
```
gcc -g -Wall -DTRACE -o server server.c errlib.c sockwrap.c
./server -a 'port_number' e.g. ./server -a 1500
```

### Client executable => open terminal go to the project folder socket_tcp/ClientFolder and type
```
gcc -g -Wall -DTRACE -o client client.c errlib.c sockwrap.c  
./client 'address' 'port_number' e.g. ./client 127.0.0.1 1500
```
