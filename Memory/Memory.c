#include "Memory.h"
int server;
char* ip;
char* port;
char* seed_ip;
char* seed_port;
int main(int argc, char **argv) {

	memory_init();

	pthread_t threadClient;
	pthread_t threadAutoJournal;
	pthread_t threadGossip;

	pthread_create(&threadGossip,NULL,auto_gossip,NULL);
	pthread_detach(threadGossip);

	pthread_create(&threadClient, NULL, listen_client, NULL);
	pthread_detach(threadClient);

	pthread_create(&threadAutoJournal, NULL, execute_journal, NULL);
	pthread_detach(threadAutoJournal);

	start_API(logger);

	kill_memory();
	return 0;
}

void memory_init(){
		load_config();

		pthread_mutex_init(&MUTEX_CONFIG,NULL);
		pthread_mutex_init(&MUTEX_MEM,NULL);
		pthread_mutex_init(&MUTEX_GOSSIP,NULL);
		pthread_mutex_init(&MUTEX_BITMAP,NULL);
		pthread_mutex_init(&MUTEX_JOURNAL,NULL);
		sem_init(&MAX_CONNECTIONS_KERNEL,0,get_max_conexiones());
		iniciar_logger();
		iniciar_logger_output();

		segmentList = list_create();

		gossip_table = list_create();


		ip = get_ip();
		port = get_port();

		add_to_gossip_table(ip,port,get_mem_number(),logger);

		THEGREATMALLOC();

}

void kill_memory(){
	list_destroy_and_destroy_elements(segmentList,segment_destroy);
	log_destroy(logger);
	free(main_memory);
	bitarray_destroy(bitmap);

	config_destroy(config);

	pthread_mutex_destroy(&MUTEX_GOSSIP);
	pthread_mutex_destroy(&MUTEX_MEM);
	pthread_mutex_destroy(&MUTEX_BITMAP);
	sem_destroy(&MAX_CONNECTIONS_KERNEL);
	pthread_mutex_destroy(&MUTEX_JOURNAL);
	pthread_mutex_destroy(&MUTEX_CONFIG);

	free(port);
	free(ip);
	gossip_table_destroy();

}

void iniciar_logger()
{
	logger = log_create("../Memory.log", "Memory", 1, LOG_LEVEL_INFO);
	log_info(logger,"--INICIANDO MEMORIA--");
}

void iniciar_logger_output(){
	output = log_create("../output.log", "Memory", 0, LOG_LEVEL_INFO);
	log_info(output,"--INICIANDO OUTPUT--");
}

void load_config() {
	config = config_create("../.config");
	if(config == NULL) {
		log_error(logger, " >> No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}
//********************************** MEMORIA PRINCIPAL ***************************
int get_frame_size(){
	int frameSize=0;
	frameSize += sizeof(uint16_t); //key
	frameSize += sizeof(uint64_t); //timestamp
	frameSize += valueSize;
	return frameSize;
}

void create_bitmap(int memSize){
	pthread_mutex_lock(&MUTEX_BITMAP);
	int bitNumbers = total_frames();
	char* bitParameter = (char*)malloc(sizeof(char)*(bitNumbers/8 + 1));
	log_info(logger,"Marcos: **%d**",total_frames());
	for(int i = 0; i<=(bitNumbers/8); i++){
		bitParameter[i] = '0';
	}

	bitParameter[(bitNumbers/8)] = '\0';
	bitmap = bitarray_create(bitParameter,strlen(bitParameter));
	pthread_mutex_unlock(&MUTEX_BITMAP);

}

int total_frames(){
	int memSize = get_tam_mem();
	int frameSize = get_frame_size();
	int bitNumbers = memSize/frameSize;
	return bitNumbers;
}

void THEGREATMALLOC(){
	get_value_size();

	int memSize = get_tam_mem();

	main_memory = malloc(memSize); //EL GRAN MALLOC

	create_bitmap(memSize);

	log_info(logger,"Memoria principal alocada");
}

uint16_t get_key_from_memory(int frame_num){
	return *((uint16_t*) (main_memory + frame_num * get_frame_size()));
}

uint64_t get_timestamp_from_memory(int frame_num){
	return *((uint64_t*) (main_memory + frame_num * get_frame_size() + sizeof(uint16_t)));
}

char* get_value_from_memory(int frame_num){
	return string_duplicate((char*) (main_memory + frame_num * get_frame_size() + sizeof(uint16_t) + sizeof(uint64_t)));
}


void insert_in_frame(uint16_t key, uint64_t timestamp, char* value, int frame_num){

	pthread_mutex_lock(&MUTEX_BITMAP);
	bitarray_set_bit(bitmap,frame_num);
	pthread_mutex_unlock(&MUTEX_BITMAP);

	pthread_mutex_lock(&MUTEX_MEM);
	void* base = main_memory + frame_num * get_frame_size();
	memcpy(base, &key, sizeof(uint16_t));
	base += sizeof(uint16_t);
	memcpy(base, &timestamp, sizeof(uint64_t));
	base += sizeof(uint64_t);
	memcpy(base, value, strlen(value) + 1);
	pthread_mutex_unlock(&MUTEX_MEM);

	log_info(output,"Se inserto el value: **%s** con la key: **%d** en el frame: **%d**",value,(int)key,frame_num);

}

void modify_in_frame(char* value, int frame_num){
	pthread_mutex_lock(&MUTEX_MEM);
	void* base = main_memory + frame_num * get_frame_size() + sizeof(uint16_t);
	uint64_t timestamp = get_timestamp();
	memcpy(base, &timestamp, sizeof(uint64_t));
	base += sizeof(uint64_t);
	memcpy(base, value, strlen(value) + 1);
	pthread_mutex_unlock(&MUTEX_MEM);

	log_info(output,"Se atcualizo el marco **%d** al value: **%s**",frame_num,value);

}

int find_free_frame(){
	int i=0,free_frame = -1;
	pthread_mutex_lock(&MUTEX_BITMAP);
	for(i=0;i<total_frames();i++){
		if(bitarray_test_bit(bitmap,i) == 0){
			free_frame = i;
			break;
		}
	}
	pthread_mutex_unlock(&MUTEX_BITMAP);
	return free_frame;
}

int frame_available_in_mem(){
	int bitNumbers = total_frames();
	int i=0;
	pthread_mutex_lock(&MUTEX_BITMAP);
	for(i=0;i<bitNumbers;i++){
		if(bitarray_test_bit(bitmap,i) == 0){
			pthread_mutex_unlock(&MUTEX_BITMAP);
			return 1;
		}
	}
	pthread_mutex_unlock(&MUTEX_BITMAP);
	return 0;
}

void free_frame(int frame){
	pthread_mutex_lock(&MUTEX_BITMAP);
	bitarray_clean_bit(bitmap,frame);
	pthread_mutex_unlock(&MUTEX_BITMAP);
}

//************************************ FIN MEMORIA PRINCIPAL *****************************************
/************************************** CLIENTE *********************************/
void* attend_client(void* socket) {
	int cliSocket = (int)socket;
	e_request_code rc = recv_req_code(cliSocket);
	switch (rc) {
	case REQUEST_QUERY:
		process_query_from_client(cliSocket);
		break;
	case REQUEST_GOSSIP:
		execute_gossip_server(cliSocket,logger,MUTEX_GOSSIP);
		break;
	case REQUEST_JOURNAL:
		pthread_mutex_lock(&MUTEX_JOURNAL);
		journalM();
		pthread_mutex_unlock(&MUTEX_JOURNAL);
		send_res_code(cliSocket, RESPONSE_SUCCESS);
		break;
	default:
		break;
	}
	close(cliSocket);
	sem_post(&MAX_CONNECTIONS_KERNEL);
	return NULL;

}

void *listen_client() {

	int socket = createServer(ip,port);
	log_error(logger,"Servidor creado en: %s - %s",ip,port);
	if(socket == -1) {
		printf("No se pudo crear el servidor\n");
		exit(1);
	}

	while(true) {
		int cliSocket = connectToClient(socket);

		if(cliSocket == -1) {
			log_warning(logger,"No se pudo conectar con el cliente\n");
		}else{
			sem_wait(&MAX_CONNECTIONS_KERNEL);
			pthread_t client;
			pthread_create(&client,NULL,attend_client,cliSocket);
			pthread_detach(client);
		}
//		e_request_code rc = attend_client(cliSocket);

	}
}

void process_query_from_client(int client) {
	e_query opCode;
	recv(client, &opCode, sizeof(opCode), 0);

	char *table, *value,*part, *compTime;
	int status;
	uint16_t key;
	char *consType;
	metadata* aMD;
	t_list* metadata_list;

	switch(opCode) {
		case QUERY_SELECT:
			table = recv_str(client);
			key = recv_int(client);
			log_info(logger,"Recibi un SELECT de tabla %s y key %d",table,(int)key);
			char *response = selectM(table, key);
			log_info(output,"El SELECT %s %d devuelve %s",table,key,response);
			free(table);
			if(response != NULL) {
				send_res_code(client, RESPONSE_SUCCESS);
				send_str(client, response);
				free(response);
			} else {
				send_res_code(client, RESPONSE_ERROR);
			}
			break;
		case QUERY_INSERT:
			table = recv_str(client);
			key = recv_int(client);
			value = recv_str(client);
			log_info(logger,"Recibi un INSERT de tabla %s, key %d y value %s",table,(int)key,value);
			log_info(output,"Recibi un INSERT de tabla %s, key %d y value %s",table,(int)key,value);
			pthread_mutex_lock(&MUTEX_JOURNAL);
			status = insertM(table, key, value);
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			free(value);
			free(table);
			switch(status) {
				case 0: send_res_code(client, RESPONSE_SUCCESS); break;
				case 1: send_res_code(client, RESPONSE_ERROR); break;
				case 2: send_res_code(client, RESPONSE_FULL); break;
			}
			break;

		case QUERY_CREATE:
			table = recv_str(client);
			consType = recv_str(client);
			part = recv_str(client);
			compTime = recv_str(client);
			log_info(logger,"Recibi un CREATE de tabla %s",table);
			status = createM(table, consType, part, compTime);
			free(table);
			free(consType);
			free(part);
			free(compTime);
			if(status == 0)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);

			break;

		case QUERY_DESCRIBE:

			table = recv_str(client);
			log_info(logger,"Recibi un DESCRIBE");
			if(strcmp(table, "") != 0) {
				metadata_list = describeM(table);
				if(metadata_list != NULL) {
					send_res_code(client, RESPONSE_SUCCESS);
					aMD = list_get(metadata_list,0);
					send_str(client,aMD->tableName);
					send_str(client,aMD->consType);
					send_str(client,aMD->partNum);
					send_str(client,aMD->compTime);
					list_destroy_and_destroy_elements(metadata_list,metadata_destroy);
				} else {
					send_res_code(client, RESPONSE_ERROR);
				}
			}else {
				t_list *metadata_list = describeM(NULL);
				if(metadata_list != NULL){
					int tCount = list_size(metadata_list);

					send_res_code(client, RESPONSE_SUCCESS);
					send_int(client, tCount);
					for(int i = 0; i<tCount; i++) {
						aMD = list_get(metadata_list,i);
						send_str(client,aMD->tableName);
						send_str(client,aMD->consType);
						send_str(client,aMD->partNum);
						send_str(client,aMD->compTime);
					}
					list_destroy_and_destroy_elements(metadata_list,metadata_destroy);
				}
				else {
					send_res_code(client, RESPONSE_ERROR);
				}
			}
			//free(table);
			break;
		case QUERY_DROP:
			table = recv_str(client);
			status = dropM(table);
			log_info(logger,"Recibi un DROP de tabla %s,",table);
			free(table);
			if(status == 0)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);
			break;
		default:
			break;
	}
}

/************************** FIN CLIENTE ******************************************/

void* auto_gossip(){
	log_info(logger,"[GOSSIPING]: Iniciando Gossiping");

	while(true){
		execute_gossip_client(logger,port,MUTEX_GOSSIP);
		//log_info(logger,"************** %d ************",list_size(gossip_table));
		log_info(logger,"[GOSSIPING]: La tabla de gossip es: ");
		log_gossip_table();
		sleep(get_retardo_gossip()/1000);
	}
}
void print_mem(void* mem){
	memory* memo = (memory*) mem;
	log_info(logger,"[GOSSIPING]: Memoria %d: %s - %s",memo->memory_number,memo->memory_ip,memo->memory_port);
}
void log_gossip_table(){
	list_iterate(gossip_table,print_mem);
}


void start_API(t_log *logger){

	char *input;
	input = readline(">");
	while(strcmp("", input)) {
		add_history(input);
		processQuery(input, logger);
		free(input);
		input = readline(">");
	}
	free(input);
}

e_query processQuery(char *query, t_log *logger) {
	int insertResult;
	e_query queryType;

	t_list *args = parseQuery(query); //guardas en el vector args la query

	int invalidQuery = validateQuerySyntax(args); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		list_destroy_and_destroy_elements(args,string_destroy_char);
		return queryError();
	}

	queryType = getQueryType(list_get(args,0));
	switch(queryType) {

		case QUERY_SELECT:

			log_info(logger, "Recibi un SELECT %s %s", (char*) list_get(args,1), (char*) list_get(args,2));

			char *value = selectM( (char*) list_get(args,1), atoi(list_get(args,2)));
			log_info(logger,"El valor retornado es: ** %s **",value);
			log_info(output,"El SELECT %s %d devuelve %s",(char*) list_get(args,1),atoi(list_get(args,2)),value);
			free(value);
			log_info(logger,"borraresto");
			break;

		case QUERY_INSERT:
			log_info(logger, "Recibi un INSERT %s %s %s", (char*) list_get(args,1), (char*) list_get(args,2), (char*) list_get(args,3));
			pthread_mutex_lock(&MUTEX_JOURNAL);
			insertResult = insertM(list_get(args,1), atoi(list_get(args,2)), list_get(args,3));
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			if(insertResult == -1){
				log_error(logger,"No se puedo insertar un valor");
			}else{
				log_info(logger,"Valor insertado con exito");
			}

			break;

		case QUERY_CREATE:

			log_info(logger, "Recibi un CREATE %s %s %s %s", (char*) list_get(args,1), (char*) list_get(args,2),(char*) list_get(args,3), (char*) list_get(args,4));
			createM(list_get(args,1), list_get(args,2), list_get(args,3), list_get(args,4));
			break;

		case QUERY_DESCRIBE:
			log_info(logger, "Recibi un DESCRIBE %s", (char*) list_get(args,1));
			t_list* md = describeM(list_get(args,1));
			//TODO Describe por consola
			if(md!=NULL)
				list_destroy_and_destroy_elements(md,metadata_destroy);

			break;

		case QUERY_DROP:

			log_info(logger, "Recibi un DROP %s", (char*) list_get(args,1));

			dropM(list_get(args,1));

			break;

		case QUERY_JOURNAL:

			log_info(logger, "Recibi un JOURNAL");
			pthread_mutex_lock(&MUTEX_JOURNAL);
			journalM();
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			break;

		default:
			return queryError(logger);

	}

	list_destroy_and_destroy_elements(args,string_destroy_char);
	return queryType;
}


page* search_page(segment* aSegment,uint16_t aKey){
	bool isKey(void* aPage){
			return get_key_from_memory(((page*) aPage)->frame_num) == aKey;
		}
		return list_find(aSegment->page_list,isKey);
}

segment* search_segment(char* segmentID){
	bool isId(void* aSegment){
		return strcasecmp(((segment*) aSegment)->segment_id,segmentID)==0;
	}
	return list_find(segmentList,isId);
}

int memory_full(){
	return list_all_satisfy(segmentList,segment_full);
}

segment* segment_init(){
	segment* memorySegment= segment_create();
	memorySegment->page_list = list_create();
	list_add(segmentList,memorySegment);
	log_info(logger,"Nuevo segmento añadido a la tabla.");
	return memorySegment;
}

void remove_segment(segment* aSeg){
	bool isSegment(void* aSegment){
		return strcasecmp(((segment*) aSegment)->segment_id,aSeg->segment_id)==0;
	}
	list_remove_and_destroy_by_condition(segmentList,isSegment,segment_destroy);
}

void load_page_to_segment(uint16_t key, segment* segmentFound, char* value, int modified) {
	int frame_num = find_free_frame();
	segment_add_page(segmentFound, frame_num, modified);
	insert_in_frame(key, get_timestamp(), value, frame_num);
	log_info(logger, "Se agrego la pagina con el nuevo valor a segmento y memoria al frame: %d.",frame_num);
}

void* execute_journal(){
	while(true){
		sleep(get_retardo_journal()/1000);
		log_info(logger, "[JOURNAL]: Iniciando Journal automatico.");
		pthread_mutex_lock(&MUTEX_JOURNAL);
		journalM();
		pthread_mutex_unlock(&MUTEX_JOURNAL);
	}
}
/*********************************** QUERYS ******************************************/
int journalM(){
	log_info(output,"[JOURNAL]: Ejecutando Journal");
	int errorFS=0;
	void journal_segment(void* aSegment){
		segment* s = (segment*) aSegment;
		void journal_page(void* aPage){
			page*p = (page*) aPage;
			if((p)->isModified){
				char *value = get_value_from_memory(p->frame_num);
				if(send_insert_to_FS(s->segment_id,get_key_from_memory(p->frame_num),get_timestamp_from_memory(p->frame_num),value,logger) == -2){
					errorFS = 1;
				}else{
					p->isModified = 0;
				}
				free(value);
			}
		}
		list_iterate(s->page_list,journal_page);
	}


	list_iterate(segmentList,journal_segment);

	if(errorFS != 1){
		//vaciar segment list
		list_clean_and_destroy_elements(segmentList,segment_destroy);
		//setear el bitmap en 0 de toda la mem
		for(int i=0;i<total_frames();i++){
			free_frame(i);
		}
		log_info(logger,"[JOURNAL]: Journal ejecutado");
	}else{
		log_error(logger,"[JOURNAL]: Error al conectar a FS, no se realiza el Journal");
		return -1;
	}
	return 0;
}

int insertM(char* segmentID, uint16_t key, char* value){
	segment* segmentFound = search_segment(segmentID);

	if(segmentFound == NULL){
		log_warning(logger,"No se encontro el segmento buscado, creando nuevo segmento.");
		segmentFound = segment_init();
		segmentFound->segment_id = string_duplicate(segmentID);
	}else{
		log_info(logger,"Se encontro el segmento buscado.");
	}

	page* pageFound = search_page(segmentFound,key);

	if(pageFound != NULL){
		log_info(logger,"Se encontro la pagina con la key buscado, actualizando al valor %s en el frame: %d.",value,pageFound->frame_num);
		modify_in_frame(value,pageFound->frame_num);
		pageFound->isModified=1;
		pageFound->last_time_used=get_timestamp();
		return 0;
	}
	else{
		log_info(logger,"No se encontro la pagina con la key buscado,creando pagina con valor %s.", value);
		if(frame_available_in_mem()){
			load_page_to_segment(key, segmentFound, value, 1);
		}else{
			if(memory_full()){
				log_info(logger,"La memoria esta full, ejecutando Journal");
				log_info(output,"La memoria esta full");
				if(journalM() == -1){
					log_error(logger,"No se pudo realizar el Journal, cancelando Insert");
					return -1;
				}
				return insertM(segmentID, key, value);
			}else{
				execute_replacement(key,value,segmentFound,1);
			}
		}
		return 0 ;
	}


}

char* selectM(char* segmentID, uint16_t key){
	pthread_mutex_lock(&MUTEX_JOURNAL);
	int segmentCreated = 0;

	segment* segmentFound = search_segment(segmentID);

	if(segmentFound == NULL){
		log_warning(logger,"No se encontro el segmento buscado, creando nuevo segmento.");
		segmentFound = segment_init();
		segmentFound->segment_id = string_duplicate(segmentID);
		segmentCreated = 1;
	}else{
		log_info(logger,"Se encontro el segmento buscado.");
	}

	page* pageFound = search_page(segmentFound,key);

	if(pageFound != NULL){
		pageFound->last_time_used=get_timestamp();
		log_info(logger,"Se encontro la pagina con el key buscado, retornando el valor.");
		pthread_mutex_unlock(&MUTEX_JOURNAL);
		return get_value_from_memory(pageFound->frame_num);
	}else{
		log_warning(logger,"No se encontro la pagina con el key buscado, consultando a FS.");
		char* value = send_select_to_FS(segmentID,key,logger);

		if(value!=NULL){
			if(frame_available_in_mem()){
				load_page_to_segment(key, segmentFound, value, 0);
			}else{
				if(memory_full()){
					log_info(logger,"La memoria esta full, ejecutando Journal");
					log_info(output,"La memoria esta full");
					segment* segmentDup = segment_duplicate(segmentFound);
					if(journalM() == -1){
						log_error(logger,"No se pudo realizar el Journal, cancelando Select");
						segment_destroy(segmentDup);
						pthread_mutex_unlock(&MUTEX_JOURNAL);
						return NULL;
					}
					list_add(segmentList,segmentDup);
					load_page_to_segment(key, segmentDup, value, 0);
				}else{
					execute_replacement(key,value,segmentFound,0);
				}
			}
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			return value;
		}else{
			if(segmentCreated){
				remove_segment(segmentFound);
			}
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			return NULL;
		}
	}


}

int createM(char* segmentID,char* consistency ,char *partition_num, char *compaction_time){
	/*ENVIAR AL FS OPERACION PARA CREAR TABLA*/
	pthread_mutex_lock(&MUTEX_JOURNAL);
	int status = send_create_to_FS(segmentID, consistency, partition_num, compaction_time, logger);
	pthread_mutex_unlock(&MUTEX_JOURNAL);
	return status;
}

t_list* describeM(char *table){
	pthread_mutex_lock(&MUTEX_JOURNAL);
	t_list* md = send_describe_to_FS(table,logger);
	pthread_mutex_unlock(&MUTEX_JOURNAL);
	return md;
}

int dropM(char* segment_id){
	pthread_mutex_lock(&MUTEX_JOURNAL);
	segment* segmentFound = search_segment(segment_id);

	int FSresult = send_drop_to_FS(segment_id, logger);

	if(FSresult == 0){
		if(segmentFound != NULL){
			delete_segment_from_mem(segmentFound);
			remove_delete_segment(segmentFound);
			log_info(logger,"Se elimino el segmento y se libero la memoria");
		}else{
			log_info(logger,"No se encontro el segmento a borrar.");
			pthread_mutex_unlock(&MUTEX_JOURNAL);
			return -1;
		}
	}else{
		log_error(logger,"No se elimina el segmento ni se libera la memoria.");
	}
	pthread_mutex_unlock(&MUTEX_JOURNAL);
	return 0;
}
/**************************** FIN QUERYS *********************************************/
void delete_segment_from_mem(segment* aSegment){
	void make_frame_available(void* aPage){
		free_frame(((page*) aPage)->frame_num);
	}
	list_iterate(aSegment->page_list, make_frame_available);
}

void remove_delete_segment(segment* aSegment){
	bool isSegment(void* anotherSegment){
		return strcasecmp(((segment*)anotherSegment)->segment_id,aSegment->segment_id) == 0;
	}
	list_remove_and_destroy_by_condition(segmentList,isSegment,segment_destroy);
}

/********************************************* LRU ************************************/
void execute_replacement(uint16_t key, char* value, segment* segment_to_use, int isModified){
	log_info(logger,"Ejecutando algoritmo de reemplazo LRU");
	uint64_t min_time = get_timestamp();
	page* min_page = NULL;
	segment* min_segment = NULL;
	void re_segment(void* aSegment){
		segment* s = (segment*) aSegment;
		void searching_page(void* aPage){
			page*p = (page*) aPage;
			if(p->last_time_used < min_time && !p->isModified){
				min_time = p->last_time_used;
				min_page = p;
				min_segment = s;
			}
		}
		list_iterate(s->page_list,searching_page);
	}
	list_iterate(segmentList,re_segment);

	if(min_segment != NULL && min_page != NULL) {
		log_info(logger,"Se remueve la key %d del segmento %s \n", (int)get_key_from_memory(min_page->frame_num), min_segment->segment_id);
		log_info(output,"[LRU]: Se remueve la key %d del segmento %s", (int)get_key_from_memory(min_page->frame_num), min_segment->segment_id);
		free_frame(min_page->frame_num);
		remove_page_from_segment(min_page,min_segment);
		load_page_to_segment(key, segment_to_use, value, isModified);
	} else {
		log_error(logger, "No se encontro una pagina para reemplazar.");
	}

}
/******************************** FIN LRU *******************************************/
void get_value_size(){
	int socket;
	do {
		socket = connect_to_FS(logger);
		if(socket == -1) {
			log_warning(logger, "No se pudo obtener el value size del FS. Intentando de nuevo en 10 segundos.");
			usleep(10 * 1000 * 1000);
		}
	} while(socket == -1);
	send_req_code(socket,REQUEST_VALUESIZE);
	valueSize = recv_int(socket);
	close(socket);
	log_info(logger, "Value Size recibido de FS.");
//	valueSize = 400;
}
/******************************* DATOS VARIABLES DEL ARCHIVO CONFIG **********************/
int get_retardo_journal(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	int retard = config_get_int_value(config,"RETARDO_JOURNAL");
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return retard;
}
int get_retardo_gossip(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	int retard = config_get_int_value(config,"RETARDO_GOSSIPING");
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return retard;
}
int get_max_conexiones(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	int amount = config_get_int_value(config,"MAX_CONEXIONES_KERNEL");
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return amount;
}

int get_tam_mem(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	int size = config_get_int_value(config,"TAM_MEM");
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return size;
}

char* get_ip(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char* ip = string_duplicate(config_get_string_value(config,"IP"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return ip;
}

char* get_port(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char* port = string_duplicate(config_get_string_value(config,"PUERTO"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return port;
}

char* get_ip_fs(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char* ip = string_duplicate(config_get_string_value(config,"IP_FS"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return ip;
}

char* get_port_fs(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char* port = string_duplicate(config_get_string_value(config,"PUERTO_FS"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return port;
}

char **get_ip_seeds(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char** ips = config_get_array_value(config, "IP_SEEDS");
//			array_duplicate(config_get_array_value(config, "IP_SEEDS"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return ips;
}

char **get_port_seeds(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	char** ports = array_duplicate(config_get_array_value(config, "PUERTO_SEEDS"));
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return ports;
}

int get_mem_number(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
	int num = config_get_int_value(config,"MEMORY_NUMBER");
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return num;
}

char** array_duplicate(char** array){
	char** arrayDup = (char**) malloc(sizeof(char*)*(sizeofArray(array)+1));
	int i=0;
	for(i=0; i< sizeofArray(array);i++){
		arrayDup[i] = string_duplicate(array[i]);
	}
	arrayDup[i] = NULL;
	return arrayDup;
}


/************************** FIN DATOS VARIABLES DEL ARCHIVO CONFIG *****************************/

