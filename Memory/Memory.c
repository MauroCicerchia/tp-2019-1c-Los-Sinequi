#include "Memory.h"
int server;

int main(int argc, char **argv) {
	load_config();

	segmentList = list_create();

	iniciar_logger();

	pthread_t threadClient;

	pthread_create(&threadClient, NULL, listen_client, NULL);
	pthread_detach(threadClient);

	start_API(logger);

	list_destroy_and_destroy_elements(segmentList,segment_destroy);
	config_destroy(config);
	log_destroy(logger);

	return 0;
}

void iniciar_logger()
{
	logger = log_create("../Memory.log", "Memory", 0, LOG_LEVEL_INFO);
}

void *listen_client() {
	char *ip = config_get_string_value(config, "IP");
	char *port = config_get_string_value(config, "PUERTO");
	int socket = createServer(ip,port);
	if(socket == -1) {
		printf("No se pudo crear el servidor\n");
		exit(1);
	}
	while(true) {
		int cliSocket = connectToClient(socket);

		if(cliSocket == -1) {
			printf("No se pudo conectar con el cliente\n");
			exit(1);
		}


		e_request_code rc = recv_req_code(cliSocket);

		switch(rc) {
			case REQUEST_QUERY: process_query_from_client(cliSocket); break;
			case REQUEST_GOSSIP: break;
			case REQUEST_JOURNAL: break;
		}
	}
}

void process_query_from_client(int client) {
	e_query opCode;
	recv(client, &opCode, sizeof(opCode), 0);

	char *table, *value;
	int key, part, compTime, size;
	e_cons_type consType;

	switch(opCode) {
		case QUERY_SELECT:
			table = recv_str(client);
			key = recv_int(client);
			char *response = selectM(table, key);

			if(response != NULL) {
				send_res_code(client, RESPONSE_SUCCESS);
				send_str(client, response);
			} else {
				send_res_code(client, RESPONSE_ERROR);
			}
			break;
		case QUERY_INSERT:
			table = recv_str(client);
			key = recv_int(client);
			value = recv_str(client);
			int status = insertM(table, key, value);
			switch(status) {
				case 0: send_res_code(client, RESPONSE_SUCCESS); break;
				case 1: send_res_code(client, RESPONSE_ERROR); break;
				case 2: send_res_code(client, RESPONSE_FULL); break;
			}
			break;
		case QUERY_CREATE:
			table = recv_str(client);
			consType = getConsistencyType(recv_str(client));
			part = recv_int(client);
			compTime = recv_int(client);
//			int status = createM(table, consType, part, compTime);
			send_res_code(client, RESPONSE_SUCCESS);
			break;
		case QUERY_DESCRIBE:
			table = recv_str(client);
			if(table != NULL) {
//				table_t *t = describeM(table);
//				if(t != NULL) {
					send_res_code(client, RESPONSE_SUCCESS);
//					send_int(client, 1);
//					send_table(client, t);
//				} else {
//					send_res_code(client, RESPONSE_ERROR);
//				}
			} else {
//				t_list *tl = describeM();
//				int tCount = list_size(tl);
//				if(tCount != 0) {
					send_res_code(client, RESPONSE_SUCCESS);
//					send_int(client, tCount);
					void sendTable(void *t) {
//						send_table(client, (table_t*)t);
					}
//					list_iterate(tl, sendTable);
//				} else {
					send_res_code(client, RESPONSE_ERROR);
//				}
			}
			break;
		case QUERY_DROP:
			table = recv_str(client);
//			int status = dropM(table);
			send_res_code(client, RESPONSE_SUCCESS);
			break;
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

	char **args = parseQuery(query); //guardas en el vecor args la query

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

			insertM(args[1], atoi(args[2]), args[3]);

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

int insertM(char* segmentID, int key, char* value){

	segment* segmentFound = search_segment(segmentID);

	if(segmentFound != NULL){
		log_info(logger,"Se encontro la tabla buscada.");
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			log_info(logger,"Se encontro la pagina con el key buscado, modificando el valor.");
			strcpy(pageFound->page_data->value,value);
			pageFound->page_data->timestamp= get_timestamp();
			pageFound->isModified=1;
			return 0;
		}
		else{
			log_info(logger,"No se encontro la pagina con el key buscado, chequeando si hay paginas disponibles.");
			if(segment_Pages_Available(segmentFound)){
				segment_add_page(segmentFound,key,value);
				log_info(logger,"Se agrego la pagina con el nuevo valor.");
				return 0;
			}
			else{
				if(segment_Full(segmentFound)){
					return 2;
				}
				else{
					//ejecutarReemplazo
					return 0;
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
	return 0;
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

void load_config() {
	config = config_create("../.config");
	if(config == NULL) {
		log_error(logger, " >> No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}
