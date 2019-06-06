#include "Memory.h"
int server;

int main(int argc, char **argv) {

	segmentList = list_create();

	iniciar_logger();

	pthread_t threadClient;

	pthread_create(&threadClient, NULL, listen_client, NULL);
	pthread_detach(threadClient);

	start_API(logger);

	return 0;
}

void iniciar_logger()
{
	logger = log_create("Memory.log", "Memory", 0, LOG_LEVEL_INFO);
}

void *listen_client() {
	int socket = createServer("127.0.0.1", "64782");
	while(true) {
		int cliSocket = connectToClient(socket);

		e_query opCode;

		recv(cliSocket, &opCode, sizeof(opCode), 0);

		char *table;
		int key, size;
		e_response_code resCode;

		switch(opCode) {
			case QUERY_SELECT:

				recv(cliSocket, &size, sizeof(int), 0);
				table = (char*)malloc(size);
				recv(cliSocket, table, size, 0);
				table[strlen(table)] = '\0';
				recv(cliSocket, &size, sizeof(int), 0);
				recv(cliSocket, &key, size, 0);

				char *response = selectM(table, key);

				if(response == NULL) {
					resCode = RESPONSE_ERROR;
					send(cliSocket, &resCode, sizeof(resCode), 0);
				} else {
					resCode = RESPONSE_SUCCESS;
					send(cliSocket, &resCode, sizeof(resCode), 0);
					size = sizeof(char) * (strlen(response) + 1);
					send(cliSocket, &size, sizeof(size), 0);
					send(cliSocket, response, size, 0);
				}
				break;
		}
	}
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

e_query processQuery(char *query, t_log *logger) {

	char log_msg[100];
	e_query queryType;

	char **args = string_split(query, " "); //guardas en el vecor args la query

	queryType = getQueryType(args[0]); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args, queryType); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery)
		return queryError();

	switch(queryType) {

		case QUERY_SELECT:
//			sendMessage(server,query);
			printf("%s",selectM(args[1], atoi(args[2])));
//			queryToFileSystem(*query);
			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);

			break;

		case QUERY_INSERT:

			insertM(args[1], atoi(args[2]), args[3], logger);

			sprintf(log_msg, "Recibi un INSERT %s %s %s", args[1], args[2], args[3]);

			break;

		case QUERY_CREATE:

			//create(args[1], args[2], args[3], args[4]);

			sprintf(log_msg, "Recibi un CREATE %s %s %s %s", args[1], args[2], args[3], args[4]);

			break;

		case QUERY_DESCRIBE:

			//describe(args[1]);

			sprintf(log_msg, "Recibi un DESCRIBE %s", args[1]);

			break;

		case QUERY_DROP:

			//drop(args[1]);

			sprintf(log_msg, "Recibi un DROP %s", args[1]);

			break;

		case QUERY_JOURNAL:

			//journal();

			sprintf(log_msg, "Recibi un JOURNAL");

			break;

		default:
			return queryError(logger);

	}

	log_info(logger, log_msg);
	return queryType;
}

page* search_page(segment* aSegment,int aKey){
	bool isKey(void* aPage){
			return ((page*) aPage)->page_data->key == aKey;
		}
		return list_find(aSegment->page_list,isKey);
}

segment* search_segment(char* segmentID){
	bool isId(void* aSegment){
		return strcasecmp(((segment*) aSegment)->segment_id,segmentID)==0;
	}
	return list_find(segmentList,isId);
}

segment* segment_init(t_log* logger){

	segment* memorySegment= segment_create();
	memorySegment->page_list = list_create();
	list_add(segmentList,memorySegment);
	log_info(logger,"Nuevo segmento añadido a la tabla.");
	return memorySegment;
}

void insertM(char* segmentID, int key, char* value, t_log *logger){

	segment* segmentFound = search_segment(segmentID);

	if(segmentFound != NULL){
		log_info(logger,"Se encontro la tabla buscada.");
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			log_info(logger,"Se encontro la pagina con el key buscado, modificando el valor.");
			strcpy(pageFound->page_data->value,value);
			pageFound->page_data->timestamp= get_timestamp();
			pageFound->isModified=1;
		}
		else{
			log_info(logger,"No se encontro la pagina con el key buscado, chequeando si hay paginas disponibles.");
			if(segment_Pages_Available(segmentFound)){
				segment_add_page(segmentFound,key,value);
				log_info(logger,"Se agrego la pagina con el nuevo valor.");
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
		log_info(logger,"No se encontro la tabla buscada, creando nuevo segmento.");
		segment* newSegment = segment_init(logger);
		newSegment->segment_id = segmentID;
		segment_add_page(newSegment,key,value);
		log_info(logger,"Se agrego la pagina con el nuevo valor.");

		//ACA HABRIA QUE CONSIDERAR QUE UN SEGMENTO NO PUEDA TENER PAGINAS POR MEMORIA PRINCIPAL LLENA,
		//POR EL MOMENTO NO NOS AFECTA
	}

}


char* selectM(char* segmentID, int key){
	//Busca si existe una pagina con esta key
	//Develve el valor asociado

	segment* segmentFound = search_segment(segmentID);


	if(segmentFound != NULL){
		log_info(logger,"Se encontro la tabla buscada.");
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			log_info(logger,"Se encontro la pagina con el key buscado, retornando el valor.");
			return pageFound->page_data->value;
		}else{
			log_info(logger,"No se encontro la pagina con el key buscado, consultando a FS.");
			return NULL;
			//value = fileSystem.solicitarValor(key);
			//if(hayLugar)agregar_pagina(segmentID,key,value)
			//return value
		}
	}
	else{
		log_error(logger,"No existe la tabla ingresada");
	}

	return NULL;
}

//void createM(char segmentID*,/*consistencia,*/int partition_num, int compaction_time){
	/*ENVIAR AL FS OPERACION PARA CREAR TABLA*/

int get_timestamp(){
	return (int)time(NULL);
}

