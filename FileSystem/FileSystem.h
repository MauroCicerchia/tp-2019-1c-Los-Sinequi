#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include"console.h"
#include"server.h"

e_query processQuery(char *, t_log*);

void iniciar_logger(t_log **logger)
{
	*logger = log_create("FileSystem.log", "FileSystem", 1, LOG_LEVEL_INFO);
}

void iniciar_servidor(t_log *logger) {
	char *queryFromClient;

	queryFromClient = (char*) malloc(sizeof(char) * PACKAGESIZE);

	printf("Iniciando servidor...\n");

	int server = createServer();

	printf("Esperando conexion con cliente...\n");

	int client = connectToClient(server);

	printf("Cliente conectado\n");

	int readStatus = readQueryFromClient(client, queryFromClient);
	while(readStatus != -1) {
		processQuery(queryFromClient, logger);
		readStatus = readQueryFromClient(client, queryFromClient);
	}

	printf("Cliente desconectado\n");

	closeServer(server);
}
