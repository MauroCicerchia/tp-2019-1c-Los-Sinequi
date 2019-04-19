#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include"console.h"
#include"server.h"

e_query processQuery(char *, t_log*);

void iniciar_logger(t_log **logger)
{
	*logger = log_create("Memory.log", "Memory", 1, LOG_LEVEL_INFO);
}

void iniciar_servidor(t_log *logger) {
	char *queryFromClient;

	queryFromClient = (char*) malloc(sizeof(char) * PACKAGESIZE);

	int server = createServer();

	//TODO
	int client = connectToClient(server);
	int readStatus = readQueryFromClient(client, queryFromClient);
	while(readStatus != -1) {
		processQuery(queryFromClient, logger);
		readStatus = readQueryFromClient(client, queryFromClient);
	}

	closeServer(server);
}
