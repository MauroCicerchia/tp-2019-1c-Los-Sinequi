#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<commons/collections/list.h>

#define PUERTO "8080"
#define BACKLOG SOMAXCONN
#define PACKAGESIZE 1024

int createServer();
int connectToClient(int);
int readQueryFromClient(int, char*);
void closeServer(int);
