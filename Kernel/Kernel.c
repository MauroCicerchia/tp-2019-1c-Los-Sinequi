#include"Kernel.h"

#define MP 1

int server, nroProcesos = 0;
t_list *memories, *tables;
t_config *config;
t_queue *new, *ready;
t_process *exec[MP];
sem_t MUTEX_NEW, MUTEX_READY, MUTEX_MEMORIES, MUTEX_TABLES, PROC_PEND_NEW, MAX_PROC_READY, PROC_PEND_READY, FREE_PROC[MP];
t_log *logger;
pthread_t threadNewReady, threadsExec[MP];

int main(int argc, char **argv) {

	init_kernel();

	display_memories();

	output_describe("T1", CONS_EC, 3, 3000);

	pthread_create(&threadNewReady, NULL, new_to_ready, NULL);
	pthread_detach(threadNewReady);
	for(int i = 0; i < MP; i++) {
		pthread_create(&threadsExec[i], NULL, processor_execute, (void*)i);
		pthread_detach(threadsExec[i]);
	}

	start_API(logger);

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
	tables = list_create();
	sem_init(&MUTEX_NEW, 0, 1);
	sem_init(&MUTEX_READY, 0, 1);
	sem_init(&MUTEX_MEMORIES, 0, 1);
	sem_init(&MUTEX_TABLES, 0, 1);
	sem_init(&PROC_PEND_NEW, 0, 0);
	sem_init(&PROC_PEND_READY, 0, 0);
	sem_init(&MAX_PROC_READY, 0, get_multiprogramming_degree());
	for(int i = 0; i < MP; i++) {
		sem_init(&FREE_PROC[i], 0, 1);
	}
	init_memory();
}

void kill_kernel() {
	log_info(logger, "Terminando Kernel");
	log_info(logger, "------------------------------------------------------------");
	config_destroy(config);
	log_destroy(logger);
//	pthread_cancel(threadNewReady);
//	for(int i = 0; i < MP; i++) {
//		pthread_cancel(threadsExec[i]);
//	}
	queue_destroy_and_destroy_elements(new, process_destroy);
	queue_destroy_and_destroy_elements(ready, process_destroy);
	list_destroy_and_destroy_elements(memories, memory_destroy);
	sem_destroy(&MUTEX_NEW);
	sem_destroy(&MUTEX_READY);
	sem_destroy(&PROC_PEND_NEW);
	sem_destroy(&PROC_PEND_READY);
	sem_destroy(&MAX_PROC_READY);
	for(int i = 0; i < MP; i++) {
		sem_destroy(&FREE_PROC[i]);
	}
}

e_query processQuery(char *query) {

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
			sprintf(log_msg, " >> Recibi un SELECT %s %s", args[1], args[2]);
			break;

		case QUERY_INSERT:
			isQuery = 1;
			sprintf(log_msg, " >> Recibi un INSERT %s %s %s", args[1], args[2], args[3]);
			break;

		case QUERY_CREATE:
			isQuery = 1;
			sprintf(log_msg, " >> Recibi un CREATE %s %s %s %s", args[1], args[2], args[3], args[4]);
			break;

		case QUERY_DESCRIBE:
			isQuery = 1;
			sprintf(log_msg, " >> Recibi un DESCRIBE %s", args[1]);
			break;

		case QUERY_DROP:
			isQuery = 1;
			sprintf(log_msg, " >> Recibi un DROP %s", args[1]);
			break;

		case QUERY_JOURNAL:
			sprintf(log_msg, " >> Recibi un JOURNAL");
			break;

		case QUERY_ADD:
			add_memory_to_cons_type(atoi(args[2]), getConsistencyType(args[4]));
			sprintf(log_msg, " >> Recibi un ADD MEMORY %s TO %s", args[2], args[4]);
			break;

		case QUERY_RUN:
			if(!read_lql_file(args[1]))
				return queryError();
			sprintf(log_msg, " >> Recibi un RUN %s", args[1]);
			break;

		case QUERY_METRICS:
			sprintf(log_msg, " >> Recibi un METRICS");
			break;

		default:
			return queryError(logger);
	}

	log_info(logger, log_msg);

	if(isQuery) {
		t_query *newQuery = query_create(queryType, args);
		t_list *queryList = list_create();
		list_add(queryList, (void*)newQuery);
		t_process *newProcess = process_create(nroProcesos, queryList);
		nroProcesos++;

		add_process_to_new(newProcess);
	}
	return queryType;
}

int read_lql_file(char *path) {
	char buffer[200];
	FILE *lql = fopen(path, "rt");
	if(lql == NULL) {
		char msg[200];
		sprintf(msg, " >> No se ha podido abrir el archivo %s", path);
		log_error(logger, msg);
		return 0;
	}

	t_list *fileQuerys = list_create();

	while(fgets(buffer, sizeof(buffer), lql)) {
		char **args = parseQuery(buffer);
		e_query queryType = getQueryType(args[0]);
		t_query *currentQuery = query_create(queryType, args);
		list_add(fileQuerys, (void*)currentQuery);
	}

	t_process *newProcess = process_create(nroProcesos, fileQuerys);
	nroProcesos++;
	add_process_to_new(newProcess);

	fclose(lql);
	return 1;
}

void add_process_to_new(t_process* process) {
	char msg[50];
	sem_wait(&MUTEX_NEW);
	queue_push(new, (void*) process);
	sprintf(msg, " >> Proceso %d agregado a la cola de NEW", process->pid);
	log_info(logger, msg);
	sem_post(&MUTEX_NEW);
	sem_post(&PROC_PEND_NEW);

}

void *new_to_ready() {
	char msg[50];
	while(true) {
		sem_wait(&MAX_PROC_READY);
		sem_wait(&PROC_PEND_NEW);
		sem_wait(&MUTEX_NEW);
		void *p = queue_pop(new);
		sem_post(&MUTEX_NEW);
		add_process_to_ready(p);
		sprintf(msg, " >> Proceso %d agregado a la cola de READY", ((t_process*)p)->pid);
		log_info(logger, msg);
	}
}

void add_process_to_ready(t_process *process) {
	sem_wait(&MUTEX_READY);
	queue_push(ready, process);
	sem_post(&MUTEX_READY);
	sem_post(&PROC_PEND_READY);
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
	char msg[50];
	sprintf(msg, " >> Proceso %d ejecutando en procesador %d", exec[processor]->pid, processor);
	log_info(logger, msg);
}

void *processor_execute(void *p) {
	char msg[50];
	int processor = (int)p;
	if(processor >= MP)
		return NULL;
	while(true) {
		ready_to_exec(processor);

		for(int i = 0; i < get_quantum(); i++) {
			if(process_finished(exec[processor]))
				break;
			t_query *nextQuery = process_next_query(exec[processor]);

			if(getQueryType(nextQuery->args[0]) == QUERY_ERROR || validateQuerySyntax(nextQuery->args, nextQuery->queryType) == 0) {
				sprintf(msg, " >> Error al ejecutar el proceso %d en la linea %d", exec[processor]->pid, exec[processor]->pc);
				log_error(logger, msg);
				exec[processor]->pc = process_length(exec[processor]);
				break;
			} else {
				execute_query(nextQuery);
				sleep(get_execution_delay() / 1000);
			}
		}

		if(process_finished(exec[processor])) {
			sprintf(msg, " >> Terminando proceso %d", exec[processor]->pid);
			log_info(logger, msg);
			process_destroy(exec[processor]);
			sem_post(&MAX_PROC_READY);
		} else {
			add_process_to_ready(exec[processor]);
		}
		sem_post(&FREE_PROC[processor]);
	}
	return NULL;
}

void execute_query(t_query *query) {
	switch(query->queryType) {
		case QUERY_SELECT: qSelect(query->args, logger); log_info(logger, " >> Ejecute un SELECT %s %s", query->args[1], query->args[2]); break;
		case QUERY_INSERT: qInsert(query->args, logger); log_info(logger, " >> Ejecute un INSERT %s %s \"%s\"", query->args[1], query->args[2], query->args[3]); break;
		case QUERY_CREATE: qCreate(query->args, logger); log_info(logger, " >> Ejecute un CREATE %s %s %s %s", query->args[1], query->args[2], query->args[3], query->args[4]); break;
		case QUERY_DESCRIBE: qDescribe(query->args, logger); log_info(logger, " >> Ejecute un DESCRIBE %s", query->args[1]); break;
		case QUERY_DROP: qDrop(query->args, logger); log_info(logger, " >> Ejecute un DROP %s", query->args[1]); break;
		default: break;
	}
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
//	enviar REQUEST_GOSSIP
//	recibir RESPONSE_SUCCESS cant_memorias sizeip ip size port port n veces

	//Mock
	t_memory *mem = memory_create(get_memory_ip(), get_memory_port());
	mem->mid = 1;
	memory_add_cons_type(mem, CONS_SC);
	sem_wait(&MUTEX_MEMORIES);
	list_add(memories, (void*) mem);
	sem_post(&MUTEX_MEMORIES);
}

void display_memories() {
	printf("MID	IP		PORT	CONS\n");

	void display_memory(void *memory) {
		printf("%d	%s	%s\n", ((t_memory*)memory)->mid, ((t_memory*)memory)->ip, ((t_memory*)memory)->port);
	}

	sem_wait(&MUTEX_MEMORIES);
	list_iterate(memories, display_memory);
	sem_post(&MUTEX_MEMORIES);

}

void add_memory_to_cons_type(int memid, e_cons_type consType) {
	bool findMemByID(void *mem) {
		return ((t_memory*) mem)->mid == memid;
	}
	sem_wait(&MUTEX_MEMORIES);
	t_memory *selectedMem = list_find(memories, findMemByID);
	sem_post(&MUTEX_MEMORIES);
	if(selectedMem != NULL)
		memory_add_cons_type(selectedMem, consType);
}

t_memory *get_memory_of_cons_type(e_cons_type consType) {
	bool isConsTypeMem(void *mem) {
		return memory_is_cons_type((t_memory*) mem, consType);
	}
	return (t_memory*) list_find(memories, isConsTypeMem);
}

t_memory *get_memory_for_query(t_table *t, uint16_t key) {
	//TODO resolver obtener memoria segun criterio
	t_memory *m = NULL;
	switch(t->consType) {
		case CONS_SC: /*m = get_sc_memory_for_table(t);*/ break;
		case CONS_SHC: /*m = get_shc_memory_for_table(t, key);*/ break;
		case CONS_EC: /*m = get_ec_memory();*/ break;
	}
	return m;
}

t_table *get_table(char *id) {
	bool table_has_name(void *t) {
		return strcmp(((t_table*)t)->name, id) == 0;
	}
	return (t_table*) list_find(tables, table_has_name);
}

void add_table(t_table *t) {
	sem_wait(&MUTEX_TABLES);
	list_add(tables, t);
	sem_post(&MUTEX_TABLES);
}

void update_table(char* table, e_cons_type consType, int part, int compTime) {
	t_table* t = get_table(table);
	if(t == NULL) {
		t = table_create(table, consType, part, compTime);
	} else {
		t->consType = consType;
		t->partitions = part;
		t->compTime = compTime;
	}
}

void drop_table(char *id) {
	t_table* t = get_table(id);
	if(t == NULL) {
		bool table_has_name(void *t) {
			return strcmp(((t_table*)t)->name, id);
		}
		sem_wait(&MUTEX_TABLES);
		list_remove_and_destroy_by_condition(tables, table_has_name, table_destroy);
		sem_post(&MUTEX_TABLES);
	}
}

char *get_memory_ip() {
	return config_get_string_value(config, "MEM_IP");
}

char *get_memory_port() {
	return config_get_string_value(config, "MEM_PORT");
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
	config = config_create("../.config");
	if(config == NULL) {
		log_error(logger, " >> No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}
