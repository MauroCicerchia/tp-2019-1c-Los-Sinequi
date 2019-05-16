#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/txt.h>
#include<commons/collections/list.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/server.h>

typedef struct{
	int timeStamp;
	uint16_t key;
	char *value;
}dataSelect;

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
	while(readStatus > 0) {
		processQuery(queryFromClient, logger);
		readStatus = readQueryFromClient(client, queryFromClient);
	}

	printf("Cliente desconectado\n");

	closeServer(server);
}

void start_API(t_log *logger){

	char *input;
	input = readline(">");
	while(strcmp("", input)) {
		processQuery(input, logger);
		free(input);
		input = readline(">");

	}
}


void qinsert(char*, uint16_t,char*,int);
char* toLFSmode(int,uint16_t,char*);

char *qselect(char *, uint16_t);
void loadList(t_list *,FILE *);
char *getValue(t_list *,uint16_t);
t_list *listToDATAmode(t_list *);
bool biggerTimeStamp(dataSelect, dataSelect);
bool isLastKey(uint16_t ,void*);
void *elemToDATAmode(void *);
