# Client and Server TCP stream

#SERVER STEP
```
created socket
bind
recvfrom CLIENT
sendto CLIENT
```

#CLIENT STEP
```
created socket
sendto SERVER
recvfrom SERVER
```

# Ready

## To clone project:

```
mkdir /path/to/your/project

git clone git@bitbucket.org:darioplatania/socketC.git
```

### open terminal go to the project folder and type
```
gcc -g -Wall -DTRACE -o client client.c errlib.c sockwrap.c
gcc -g -Wall -DTRACE -o server server.c errlib.c sockwrap.c

```
### open two terminal and type
```
./server -a 'port_number' e.g. ./server -a 1500
./client 'address' 'port_number' e.g. ./client 127.0.0.1 1500
```
