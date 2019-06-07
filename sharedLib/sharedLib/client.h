#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>



#ifndef CLIENT_H
#define CLIENT_H

#define PACKAGESIZE 1024

int connectToServer();
void sendMessage(int, char*);
void sendMessages(int);
void closeConnection(int);

#endif
