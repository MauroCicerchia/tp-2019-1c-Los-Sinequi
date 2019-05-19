#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/server.h>
#include"Segment.c"
#include"Page.c"

e_query processQuery(char *, t_log*);


segment create_segment(){
	segment memorySegment= segment_create();
	page segmentPage= page_create();

	memorySegment.page_pointer=segmentPage;

	return memorySegment;
}

void load_segment(segment *memorySegment, page *segmentPage, int segmentID ){
	memorySegment->page_pointer=segmentPage;
	memorySegment->segment_id=segmentID;
}

void load_page(page *segmentPage, int pageNum, pageData* pageData,bool isModified){
	segmentPage->page_num=pageNum;
	segmentPage->page_data=pageData;
	segmentPage->isModified=isModified;
}

void load_pageData(page *segmentPage, int timestamp, int key, char* value){
	segmentPage->page_data->key=key;
	segmentPage->page_data->timestamp=timestamp;
	segmentPage->page_data->value=value;
}

void select(int segmentID,int key){

}

void iniciar_logger(t_log **logger)
{
	*logger = log_create("Memory.log", "Memory", 1, LOG_LEVEL_INFO);
}

int conectar_FS(t_log *logger) {
	return connectToServer();
}

void conectar_Kernel(t_log *logger) {
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
