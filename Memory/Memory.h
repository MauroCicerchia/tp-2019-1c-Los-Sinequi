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
#include"Segment.h"

t_list* segmentList;
e_query processQuery(char *, t_log*);

segment* search_segment(char*);
page* search_page(segment*,int);
char* selectM(char*,int);	   // (nombreTabla,key)
void insertM(char*,int,char*); // (nombreTabla,key,value)

segment* segment_init(){

	segment* memorySegment= segment_create();
	memorySegment->page_list = list_create();
	list_add(segmentList,memorySegment);

	return memorySegment;
}

int mockitoTimestamp(){
	return 4;
}

char* selectM(char* segmentID,int key){
	//Busca si existe una pagina con esta key
	//Develve el valor asociado

	segment* segmentFound = search_segment(segmentID);


	if(segmentFound != NULL){
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			return pageFound->page_data->value;
		}else{
			return NULL;
			//value = fileSystem.solicitarValor(key);
			//if(hayLugar)agregar_pagina(segmentID,key,value)
			//return value
		}
	}

	return NULL;

}

void insertM(char* segmentID, int key, char* value){

	segment* segmentFound = search_segment(segmentID);

	if(segmentFound != NULL){
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			strcpy(pageFound->page_data->value,value);
			pageFound->page_data->timestamp= mockitoTimestamp();
			pageFound->isModified=1;
		}
		else{
			if(segment_Pages_Available(segmentFound)){
				segment_add_page(segmentFound,key,value);
			}
			else{
				if(segment_Full(segmentFound)){
					//ejecutarJournal
				}
				else{
					//ejecutarReemplazo
				}
			}
		}
	}
	else{
		segment* newSegment = segment_init();
		newSegment->segment_id = segmentID;
		segment_add_page(newSegment,key,value);

		//ACA HABRIA QUE CONSIDERAR QUE UN SEGMENTO NO PUEDA TENER PAGINAS POR MEMORIA PRINCIPAL LLENA,
		//POR EL MOMENTO NO NOS AFECTA
	}

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
