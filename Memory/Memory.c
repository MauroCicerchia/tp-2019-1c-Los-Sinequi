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

//	pthread_create(&threadAutoJournal, NULL, execute_journal, NULL);
//	pthread_detach(threadAutoJournal);

	start_API(logger);

	kill_memory();
	return 0;
}

void memory_init(){
		load_config();
		iniciar_logger();

		segmentList = list_create();

		gossip_table = list_create();
		ip = config_get_string_value(config, "IP");
		port = config_get_string_value(config, "PUERTO");
		add_to_gossip_table(ip,port,config,logger);

		sem_init(&MUTEX_MEM,0,1);
		THEGREATMALLOC();

}

void kill_memory(){
	list_destroy_and_destroy_elements(segmentList,segment_destroy);
	config_destroy(config);
	log_destroy(logger);
	free(main_memory);
	bitarray_destroy(bitmap);
	sem_destroy(&MUTEX_MEM);
	gossip_table_destroy();

}

void iniciar_logger()
{
	logger = log_create("../Memory.log", "Memory", 1, LOG_LEVEL_INFO);
	log_info(logger,"--INICIANDO MEMORIA--");
}

void load_config() {
	config = config_create("../.config");
	if(config == NULL) {
		log_error(logger, " >> No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}
//********************************** Memoria Principal ***************************
int get_frame_size(){

	int frameSize=0;
	frameSize += sizeof(uint16_t); //key
	frameSize += sizeof(int); //timestamp
	frameSize += valueSize;
	return frameSize;
}

void create_bitmap(int memSize){

	int bitNumbers = total_frames();
	char* bitParameter = (char*)malloc(sizeof(char)*(bitNumbers/8 + 1));
	printf("%d",total_frames());
	for(int i = 0; i<=(bitNumbers/8); i++){
		bitParameter[i] = '0';
	}

	bitParameter[(bitNumbers/8)] = '\0';
	bitmap = bitarray_create(bitParameter,strlen(bitParameter));
	//printf("%d",bitNumbers);
}

int total_frames(){
	int memSize = config_get_int_value(config, "TAM_MEM");
	int frameSize = get_frame_size();
	int bitNumbers = memSize/frameSize;
	return bitNumbers;
}

void THEGREATMALLOC(){
	get_value_size();

	int memSize = config_get_int_value(config, "TAM_MEM");

	main_memory = malloc(memSize); //EL GRAN MALLOC

	create_bitmap(memSize);

	log_info(logger,"Memoria principal alocada");
}
//*****************************************************************************
void* attend_client(void* socket) {
	int cliSocket = *(int*) socket;
	e_request_code rc = recv_req_code(cliSocket);
	switch (rc) {
	case REQUEST_QUERY:
		process_query_from_client(cliSocket);
		break;
	case REQUEST_GOSSIP:
		execute_gossip_server(cliSocket,config,logger);
		break;
	case REQUEST_JOURNAL:
		journalM();
		send_res_code(cliSocket, RESPONSE_SUCCESS);
		break;
	default:
		break;
	}
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
			//crear hilo con ese file descriptor
			//hacer lo de recibir req code y all eso
			//y listo volvemos a empezar el while true
			// semaforo max connection
			pthread_t client;
			pthread_create(&client,NULL,attend_client,&cliSocket);
			pthread_detach(client);

		}
//		e_request_code rc = attend_client(cliSocket);

	}
}

void process_query_from_client(int client) {
	e_query opCode;
	recv(client, &opCode, sizeof(opCode), 0);

  char *table, *value,*part, *compTime;
	int key, status;
	char *consType;
  metadata* aMD;
	t_list* metadata_list;

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
			status = insertM(table, key, value);
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
			status = createM(table, consType, part, compTime);
			if(status == 0)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);

			break;

		case QUERY_DESCRIBE:

			table = recv_str(client);
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
				int tCount = list_size(metadata_list);
				if(tCount != 0) {
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
				}else {
					send_res_code(client, RESPONSE_ERROR);
				}
			}
			break;
		case QUERY_DROP:
			table = recv_str(client);
			status = dropM(table);
			if(status == 0)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);
			break;
		default:
			break;
	}
}

void* auto_gossip(){
	int delay = config_get_int_value(config,"RETARDO_GOSSIPING");
	log_info(logger,"Iniciando Gossiping");
	while(true){
		execute_gossip_client(config,logger);
		//log_info(logger,"************** %d ************",list_size(gossip_table));
		print_gossip_table();
		sleep(delay/1000);
	}
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
}

e_query processQuery(char *query, t_log *logger) {
	int insertResult;
	e_query queryType;

	t_list *args = parseQuery(query); //guardas en el vecor args la query

	int invalidQuery = validateQuerySyntax(args); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery)
		return queryError();

	queryType = getQueryType(list_get(args,0));
	switch(queryType) {

		case QUERY_SELECT:

			log_info(logger, "Recibi un SELECT %s %s", (char*) list_get(args,1), (char*) list_get(args,2));

//			sendMessage(server,query);
			printf("%s",selectM( (char*) list_get(args,1), atoi(list_get(args,2))));
//			queryToFileSystem(*query);

			break;

		case QUERY_INSERT:
			log_info(logger, "Recibi un INSERT %s %s %s", (char*) list_get(args,1), (char*) list_get(args,2), (char*) list_get(args,3));

			insertResult = insertM(list_get(args,1), atoi(list_get(args,2)), list_get(args,3));

			if(insertResult == 1){
				log_error(logger,"No se puedo insertar un valor");
			}else if(insertResult == 2){
				log_info(logger,"No se puedo cargar el valor, pues la memoria se encuentra llena");
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
			describeM(list_get(args,1));

			break;

		case QUERY_DROP:

			log_info(logger, "Recibi un DROP %s", (char*) list_get(args,1));

			dropM(list_get(args,1));

			break;

		case QUERY_JOURNAL:

			log_info(logger, "Recibi un JOURNAL");
			journalM();

			break;

		default:
			return queryError(logger);

	}

	//log_info(logger, log_msg);
	return queryType;
}

/*page* search_page(segment* aSegment,int aKey){
	bool isKey(void* aPage){
			return ((page*) aPage)->page_data->key == aKey;
		}
		return list_find(aSegment->page_list,isKey);
}*/

page* search_page(segment* aSegment,uint16_t aKey){
	bool isKey(void* aPage){
			return get_key_from_memory(((page*) aPage)->frame_num) == aKey;
		}
		return list_find(aSegment->page_list,isKey);
}

uint16_t get_key_from_memory(int frame_num){
	return *((uint16_t*) (main_memory + frame_num * get_frame_size()));
}

int get_timestamp_from_memory(int frame_num){
	return *((int*) (main_memory + frame_num * get_frame_size() + sizeof(uint16_t)));
}

char* get_value_from_memory(int frame_num){
	return string_duplicate((char*) (main_memory + frame_num * get_frame_size() + sizeof(uint16_t) + sizeof(int)));
}


void insert_in_frame(uint16_t key, int timestamp, char* value, int frame_num){

	void* base = main_memory + frame_num * get_frame_size();
	memcpy(base, &key, sizeof(uint16_t));
	base += sizeof(uint16_t);
	memcpy(base, &timestamp, sizeof(int));
	base += sizeof(int);
	memcpy(base, value, valueSize);

	bitarray_set_bit(bitmap,frame_num);

}

void modify_in_frame(char* value, int frame_num){
	void* base = main_memory + frame_num * get_frame_size() + sizeof(uint16_t);
	int timestamp = get_timestamp();
	memcpy(base, &timestamp, sizeof(int));
	base += sizeof(int);
	memcpy(base, value, valueSize);

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

int find_free_frame(){
	int i=0;
	while(bitarray_test_bit(bitmap,i) != 0){
		i++;
	}
	return i;
}

int frame_available_in_mem(){
	int memSize = config_get_int_value(config, "TAM_MEM");
	int bitNumbers = total_frames(memSize);
	int i=0;
	for(i=0;i<bitNumbers;i++){
		if(bitarray_test_bit(bitmap,i) == 0){
			return 1;
		}
	}
	return 0;
}

void load_page_to_segment(int key, segment* segmentFound, char* value, int modified) {
	int frame_num = find_free_frame();
	segment_add_page(segmentFound, frame_num, modified);
	insert_in_frame(key, get_timestamp(), value, frame_num);
	log_info(logger, "Se agrego la pagina con el nuevo valor a segmento y memoria.");
}

void* execute_journal(){
	while(true){
		sleep(config_get_int_value(config, "RETARDO_JOURNAL")/1000);
		log_info(logger, "Ejecutando Journal automatico.");
		journalM();
	}
}

void journalM(){
	void journal_segment(void* aSegment){
		segment* s = (segment*) aSegment;
		void journal_page(void* aPage){
			page*p = (page*) aPage;
			if((p)->isModified){
				send_insert_to_FS(s->segment_id,get_key_from_memory(p->frame_num),get_value_from_memory(p->frame_num),config,logger);
				p->isModified = 0;
			}
		}
		list_iterate(s->page_list,journal_page);
	}
	list_iterate(segmentList,journal_segment);
	//vaciar segment list
	list_clean_and_destroy_elements(segmentList,segment_destroy);
	//setear el bitmap en 0 de toda la mem
	for(int i=0;i<total_frames();i++){
		bitarray_clean_bit(bitmap,i);
	}
}

int insertM(char* segmentID, int key, char* value){

	if (frame_available_in_mem()){
		segment* segmentFound = search_segment(segmentID);
		if(segmentFound != NULL){
			log_info(logger,"Se encontro la tabla buscada.");
			page* pageFound = search_page(segmentFound,key);
			if(pageFound != NULL){
				log_info(logger,"Se encontro la pagina con el key buscado, modificando el valor.");
				sem_wait(&MUTEX_MEM);
				modify_in_frame(value,pageFound->frame_num);
				sem_post(&MUTEX_MEM);
				pageFound->isModified=1;
				pageFound->last_time_used=get_timestamp();
				return 0;
			}
			else{
				log_warning(logger,"No se encontro la pagina con el key buscado,creando pagina.");
				load_page_to_segment(key, segmentFound, value, 1);
				return 0 ;
			}
		}
		else{
			log_info(logger,"No se encontro la tabla buscada, creando nuevo segmento.");
			segment* newSegment = segment_init();
			newSegment->segment_id = segmentID;
			sem_wait(&MUTEX_MEM);
			load_page_to_segment(key, newSegment, value, 1);
			sem_post(&MUTEX_MEM);
			return 0;
			}
	}
	else{
		if(memory_full()){
			journalM();
			return insertM(segmentID, key, value);
		}else{
			segment* segmentFound = search_segment(segmentID);
			if(segmentFound == NULL){
				segmentFound = segment_init();
				segmentFound->segment_id = segmentID;
			}
			execute_replacement(key,value,segmentFound);

			return 0;
		}
	}
return 0;

}

char* selectM(char* segmentID, int key){
	segment* segmentFound = search_segment(segmentID);

	if(segmentFound != NULL){
		log_info(logger,"Se encontro la tabla buscada.");
		page* pageFound = search_page(segmentFound,key);
		if(pageFound != NULL){
			pageFound->last_time_used=get_timestamp();
			log_info(logger,"Se encontro la pagina con el key buscado, retornando el valor.");
			return get_value_from_memory(pageFound->frame_num);
		}else{
			log_warning(logger,"No se encontro la pagina con el key buscado, consultando a FS.");
			char* value = send_select_to_FS(segmentID,key,config,logger);
			if(value!=NULL){
				if(frame_available_in_mem()){
					sem_wait(&MUTEX_MEM);
					load_page_to_segment(key, segmentFound, value, 0);
					sem_post(&MUTEX_MEM);
				}else{
					if(memory_full()){
						journalM();
						load_page_to_segment(key, segmentFound, value, 0);
					}else{
						execute_replacement(key,value,segmentFound);
					}
				}
				return value;
			}else{
				log_error(logger,"No existe la pagina ingresada en FS");
				return NULL;
			}
		}
	}
	else{
		log_warning(logger,"No existe la tabla ingresada en memoria");
		char* value = send_select_to_FS(segmentID,key,config,logger);
		if(value!=NULL){
			segment* newSegment = segment_init();
			newSegment->segment_id = segmentID;
			if(frame_available_in_mem()){
				sem_wait(&MUTEX_MEM);
				load_page_to_segment(key, newSegment, value, 0);
				sem_post(&MUTEX_MEM);
			}else{
				if(memory_full()){
					journalM();
					sem_wait(&MUTEX_MEM);
					load_page_to_segment(key, newSegment, value, 0);
					sem_post(&MUTEX_MEM);
				}else{
					execute_replacement(key,value,newSegment);
				}
			}
			return value;
		}else{
			log_error(logger,"No existe la tabla o la pagina ingresada en FS");
		}
	}

	return NULL;
}


int createM(char* segmentID,char* consistency ,char *partition_num, char *compaction_time){
	/*ENVIAR AL FS OPERACION PARA CREAR TABLA*/

	int status = send_create_to_FS(segmentID, consistency, partition_num, compaction_time, config, logger);
	return status;
}

t_list* describeM(char *table){
	t_list* md = send_describe_to_FS(table,config,logger);
	return md;
}

int dropM(char* segment_id){

	segment* segmentFound = search_segment(segment_id);

	send_drop_to_FS(segment_id, config, logger);

	if(segmentFound != NULL){
		sem_wait(&MUTEX_MEM);
		delete_segment_from_mem(segmentFound);
		remove_delete_segment(segmentFound);
		sem_post(&MUTEX_MEM);
		log_info(logger,"Se elimino el segmento y se libero la memoria");
	}else{
		log_info(logger,"No se encontro la tabla a borrar.");
		return -1;
	}

	return 0;
}

void delete_segment_from_mem(segment* aSegment){
	void make_frame_available(void* aPage){
		bitarray_clean_bit(bitmap,((page*) aPage)->frame_num);
	}
	list_iterate(aSegment->page_list, make_frame_available);
}

void remove_delete_segment(segment* aSegment){
	bool isSegment(void* anotherSegment){
		return strcasecmp(((segment*)anotherSegment)->segment_id,aSegment->segment_id) == 0;
	}
	list_remove_and_destroy_by_condition(segmentList,isSegment,segment_destroy);
}

void execute_replacement(int key, char* value, segment* segment_to_use){
	log_info(logger,"Ejecutando algoritmo de reemplazo LRU");
	int min_time = get_timestamp();
	page* min_page;
	segment* min_segment;
	void re_segment(void* aSegment){
		segment* s = (segment*) aSegment;
		void searching_page(void* aPage){
			page*p = (page*) aPage;
			if((p)->last_time_used < min_time && !(p)->isModified){
				min_time = (p)->last_time_used;
				min_page = (p);
				min_segment = s;
			}
		}
		list_iterate(s->page_list,searching_page);
	}
	list_iterate(segmentList,re_segment);

	log_info(logger,"Se remueve la key %d del segmento %s \n", get_key_from_memory(min_page->frame_num),min_segment->segment_id);

	remove_page_from_segment(min_page,min_segment);
	sem_wait(&MUTEX_MEM);
	load_page_to_segment(key, segment_to_use, value, 0);
	sem_post(&MUTEX_MEM);

}

void get_value_size(){
	int socket = connect_to_FS(config, logger);
	send_req_code(socket,REQUEST_VALUESIZE);
	valueSize = recv_int(socket);
	close(socket);
	log_info(logger, "Value Size recibido de FS.");
//	valueSize = 255;
}

int get_timestamp(){
	return (int)time(NULL);
}



