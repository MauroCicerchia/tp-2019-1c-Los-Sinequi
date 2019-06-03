#include "Memory.h"
int server;

int main(int argc, char **argv) {

	segmentList = list_create();

	t_log *logger = NULL;
	char *input;
	//server = conectar_FS(logger);
	iniciar_logger(&logger);
	/*
	pthread_t threadKernel;
	pthread_t threadFS;

	//2 hilos diferentes
	pthread_create(&threadFS,NULL,start_API,logger);
	//pthread_create(&threadKernel,NULL,conectar_Kernel,logger);


	//pthread_join(threadKernel,NULL);
	pthread_join(threadFS,NULL);
	closeConnection(server);

//	conectar_Kernel(logger);//conectar con kernel
//	conectar_FS(logger);
//	start_API(logger);

	log_destroy(logger);
	//segment testSegment;

	//testSegment=segment_init();

*/

	insertM("tabla1",2,"EstoAnda",logger);
	printf("El valor es: %s",selectM("tabla1",2,logger));


	return 0;
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
void iniciar_logger(t_log **logger)
{
	*logger = log_create("Memory.log", "Memory", 1, LOG_LEVEL_INFO);
}

int conectar_FS(t_log *logger) {
	return connectToServer();
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
			sendMessage(server,query);
			//select(args[1], args[2]);
//			queryToFileSystem(*query);
			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);

			break;

		case QUERY_INSERT:

			//insert(args[1], args[2], args[3], args[4]);

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

segment* segment_init(){

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
			pageFound->page_data->timestamp= mockitoTimestamp();
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
		segment* newSegment = segment_init();
		newSegment->segment_id = segmentID;
		segment_add_page(newSegment,key,value);
		log_info(logger,"Se agrego la pagina con el nuevo valor.");

		//ACA HABRIA QUE CONSIDERAR QUE UN SEGMENTO NO PUEDA TENER PAGINAS POR MEMORIA PRINCIPAL LLENA,
		//POR EL MOMENTO NO NOS AFECTA
	}

}

char* selectM(char* segmentID, int key, t_log *logger){
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

