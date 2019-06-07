#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<commons/collections/list.h>

#ifndef SERVER_H
#define SERVER_H

#define BACKLOG SOMAXCONN
#define PACKAGESIZE 1024

int createServer(char*, char*);
int connectToClient(int);
int readQueryFromClient(int, char*);
void closeServer(int);

#endif
