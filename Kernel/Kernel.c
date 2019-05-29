#include"Kernel.h"

#define MP 1

int server;

t_list *memories;
t_config *config;
t_queue *new, *ready;
t_process *exec[MP];
sem_t MUTEX_NEW, MUTEX_READY, PROC_PEND_NEW, MAX_PROC_READY, PROC_PEND_READY, FREE_PROC[MP];
t_log *logger;
pthread_t threadSwitch;

int main(int argc, char **argv) {

	init_kernel();

	display_memories();

	pthread_create(&threadSwitch, NULL, new_to_ready, NULL);
	pthread_detach(threadSwitch);

	start_API(logger);

//	printf("new:%d\n", queue_size(new));
//	printf("ready:%d\n", queue_size(ready));

	kill_kernel();

	exit(0);
}

void init_kernel() {
	load_logger();
	log_info(logger, "Iniciando Kernel");
	load_config();
	new = queue_create();
	ready = queue_create();
	memories = list_create();
	sem_init(&MUTEX_NEW, 0, 1);
	sem_init(&MUTEX_READY, 0, 1);
	sem_init(&PROC_PEND_NEW, 0, 0);
	sem_init(&PROC_PEND_READY, 0, 0);
	sem_init(&MAX_PROC_READY, 0, get_multiprogramming_degree());
	int i;
	for(i = 0; i < MP; i++) {
		sem_init(&FREE_PROC[i], 0, 1);
	}
	init_memory();
}

void kill_kernel() {
	log_info(logger, "Terminando Kernel");
	config_destroy(config);
	log_destroy(logger);
//	pthread_cancel(threadNewReady);
	queue_destroy_and_destroy_elements(new, process_destroy);
	queue_destroy_and_destroy_elements(ready, process_destroy);
	list_destroy_and_destroy_elements(memories, memory_destroy);
	sem_destroy(&MUTEX_NEW);
	sem_destroy(&MUTEX_READY);
	sem_destroy(&PROC_PEND_NEW);
	sem_destroy(&PROC_PEND_READY);
	sem_destroy(&MAX_PROC_READY);
	int i;
	for(i = 0; i < MP; i++) {
		sem_destroy(&FREE_PROC[i]);
	}
}

e_query newQuery(char *query) {

	char log_msg[100];
	e_query queryType;

	char **args = validate_query_and_return_args(query);
	if(args == NULL)
		return queryError();

	queryType = getQueryType(args[0]);

	int isQuery = 0;
	switch(queryType) {
		case QUERY_SELECT:
			isQuery = 1;
			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);
			break;

		case QUERY_INSERT:
			isQuery = 1;
			sprintf(log_msg, "Recibi un INSERT %s %s %s", args[1], args[2], args[3]);
			break;

		case QUERY_CREATE:
			isQuery = 1;
			sprintf(log_msg, "Recibi un CREATE %s %s %s %s", args[1], args[2], args[3], args[4]);
			break;

		case QUERY_DESCRIBE:
			isQuery = 1;
			sprintf(log_msg, "Recibi un DESCRIBE %s", args[1]);
			break;

		case QUERY_DROP:
			isQuery = 1;
			sprintf(log_msg, "Recibi un DROP %s", args[1]);
			break;

		case QUERY_JOURNAL:
			sprintf(log_msg, "Recibi un JOURNAL");
			break;

		case QUERY_ADD:
			add_memory_to_cons_type(atoi(args[2]), getConsistencyType(args[4]));
			sprintf(log_msg, "Recibi un ADD MEMORY %s TO %s", args[2], args[4]);
			break;

		case QUERY_RUN:
			if(!read_lql_file(args[1]))
				return queryError();
			sprintf(log_msg, "Recibi un RUN %s", args[1]);
			break;

		case QUERY_METRICS:
			sprintf(log_msg, "Recibi un METRICS");
			break;

		default:
			return queryError(logger);
	}

	log_info(logger, log_msg);

	if(isQuery) {
		t_query *newQuery = query_create(queryType, args);
		t_list *queryList = list_create();
		list_add(queryList, (void*)newQuery);
		t_process *newProcess = process_create(queryList);

		add_process_to_new(newProcess);
	}
	return queryType;
}

int read_lql_file(char *path) {
	char buffer[200];
	FILE *lql = fopen(path, "rt");
	if(lql == NULL) {
		char msg[200];
		sprintf(msg, "No se ha podido abrir el archivo %s", path);
		log_error(logger, msg);
		return 0;
	}

	t_list *fileQuerys = list_create();

	while(fgets(buffer, sizeof(buffer), lql)) {
		char **args = validate_query_and_return_args(buffer);
		if(args == NULL) {
			list_destroy_and_destroy_elements(fileQuerys, query_destroy);
			fclose(lql);
			printf("El archivo no es valido.\n");
			return 0;
		}

		e_query queryType = getQueryType(args[0]);
		t_query *currentQuery = query_create(queryType, args);
		list_add(fileQuerys, (void*)currentQuery);
	}

	t_process *newProcess = process_create(fileQuerys);
	add_process_to_new(newProcess);

	fclose(lql);
	return 1;
}

void add_process_to_new(t_process* process) {
	sem_wait(&MUTEX_NEW);
	queue_push(new, (void*) process);
	sem_post(&MUTEX_NEW);
	sem_post(&PROC_PEND_NEW);
	log_info(logger, "Nuevo proceso agregado a la cola de NEW");
}

void *new_to_ready() {
	while(true) {
		sem_wait(&MAX_PROC_READY);
		sem_wait(&PROC_PEND_NEW);
		sem_wait(&MUTEX_NEW);
		void *p = queue_pop(new);
		sem_post(&MUTEX_NEW);
		sem_wait(&MUTEX_READY);
		queue_push(ready, p);
		sem_post(&MUTEX_READY);
		sem_post(&PROC_PEND_READY);
		log_info(logger, "Nuevo proceso agregado a la cola de READY");
	}
}

void ready_to_exec(int processor) {
	if(processor >= MP)
		return;
	sem_wait(&FREE_PROC[processor]);
	sem_wait(&PROC_PEND_READY);
	sem_wait(&MUTEX_READY);
	void *p = queue_pop(ready);
	sem_post(&MUTEX_READY);
	exec[processor] = (t_process*)p;
	log_info(logger, "Nuevo proceso ejecutando");
}

void init_memory() {
//	int memSocket = connect_to_memory(get_memory_ip(), get_memory_port());
	request_memory_pool(0);
//	closeConnection(memSocket);
//	t_memory *scMem = get_sc_memory();
//	printf("mid: %d", scMem->mid);
}

int connect_to_memory(char *IP, int PORT) {
	return connectToServer(IP, PORT);
}

void request_memory_pool(int memSocket) {
	//Mock
	t_memory *mem = memory_create(get_memory_ip(), get_memory_port());
	mem->mid = 1;
	memory_add_cons_type(mem, CONS_SC);
	list_add(memories, (void*) mem);
}

void display_memories() {
	printf("MID	IP		PORT	CONS\n");

	void display_memory(void *memory) {
		printf("%d	%s	%d\n", ((t_memory*)memory)->mid, ((t_memory*)memory)->ip, ((t_memory*)memory)->port);
	}

	list_iterate(memories, display_memory);

}

void add_memory_to_cons_type(int memid, e_cons_type consType) {
	bool findMemByID(void *mem) {
		return ((t_memory*) mem)->mid == memid;
	}
	t_memory *selectedMem = list_find(memories, findMemByID);
	if(selectedMem != NULL)
		memory_add_cons_type(selectedMem, consType);
}

t_memory *get_memory_of_cons_type(e_cons_type consType) {
	bool isConsTypeMem(void *mem) {
		return memory_is_cons_type((t_memory*) mem, consType);
	}
	return (t_memory*) list_find(memories, isConsTypeMem);
}

char *get_memory_ip() {
	return config_get_string_value(config, "MEM_IP");
}

int get_memory_port() {
	return config_get_int_value(config, "MEM_PORT");
}

int get_quantum() {
	return config_get_int_value(config, "QUANTUM");
}

int get_multiprogramming_degree() {
	return config_get_int_value(config, "MULT_DEGREE");
}

int get_metadata_refresh_rate() {
	return config_get_int_value(config, "MD_REFRESH_RATE");
}

int get_execution_delay() {
	return config_get_int_value(config, "EXEC_DELAY");
}

void load_logger()
{
	logger = log_create("../Kernel.log", "Kernel", 0, LOG_LEVEL_INFO);
	if(logger == NULL) {
		printf("No se pudo crear el log.\n");
		exit(-1);
	}
}

void load_config() {
	config = config_create(".config");
	if(config == NULL) {
		log_error(logger, "No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}
