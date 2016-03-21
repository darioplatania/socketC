# Client and Server UDP base example

#SERVER STEP
```
created socket
listening for udp_packet
recv to CLIENT
send to CLIENT
```

#CLIENT STEP
```
created socket
send to SERVER
recv to SERVER
```

# Ready

## To clone project:

```
mkdir /path/to/your/project

git clone git@bitbucket.org:darioplatania/socketC/socket_udp.git
```

### open terminal go to the project folder and type
```
gcc -g -Wall -DTRACE -o client_udp client_udp.c errlib.c sockwrap.c
gcc -g -Wall -DTRACE -o server_udp server_udp.c errlib.c sockwrap.c

```
### open two terminal and type
```
./server_udp 'port_number' e.g. ./server -a 1500
./client_udp 'address' 'port_number' 'datagram ' e.g. ./client 127.0.0.1 1500 helloworld
```
