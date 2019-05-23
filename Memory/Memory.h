#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/dictionary.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/server.h>
#include"Segment.h"


e_query processQuery(char *, t_log*);

segment search_segment(int);
page search_page(segment,int);

segment* segment_init(){
	segment* memorySegment= segment_create();

	memorySegment->page_list = list_create();

	return memorySegment;
}

//TODO Cargar segmento
//void load_segment(segment *memorySegment, page *segmentPage, char* segmentID ){
//	list_add(memorySegment->page_list, segmentPage);
//	memorySegment->segment_id=segmentID;
//}


char* select(char* segmentID,int key){
	//Busca si existe una pagina con esta key
	//Develve el valor asociado
	segment* segmentFound = search_segment(segmentID);
	if(segmentFound != NULL){
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			return pageFound->page_data->value;
		}else{
			//value = fileSystem.solicitarValor(key);
			//if(hayLugar)agregar_pagina(segmentID,key,value)
			//return value
		}
	}

	return NULL;

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
