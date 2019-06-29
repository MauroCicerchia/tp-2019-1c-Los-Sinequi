#include"Kernel.h"

int server, processNumber = 0, memoryNumber = 0, MP, reads = 0, writes = 0, totalOperations = 0;
float readsTime = 0.0f, writesTime = 0.0f;
t_list *memories, *tables;
t_config *config;
t_queue *new, *ready;
sem_t MUTEX_NEW, MUTEX_READY, MUTEX_MEMORIES, MUTEX_TABLES, PROC_PEND_NEW, MAX_PROC_READY, PROC_PEND_READY, MUTEX_READS, MUTEX_WRITES, MUTEX_TOTALOPS;
t_log *logger;

// TODO Validar que haya memoria antes de realizar query

int main(int argc, char **argv) {

	init_kernel();

	display_memories();

	t_table *t1 = table_create("T1", CONS_EC, 1, 1000);
	add_table(t1);

	pthread_t threadNewReady, threadsExec[MP], threadMetrics;

	pthread_create(&threadNewReady, NULL, new_to_ready, NULL);
	pthread_detach(threadNewReady);
	pthread_create(&threadMetrics, NULL, print_metrics, NULL);
	pthread_detach(threadNewReady);
	for(int i = 0; i < MP; i++) {
		pthread_create(&threadsExec[i], NULL, processor_execute, (void*)i);
		pthread_detach(threadsExec[i]);
	}

	sleep(1);

	start_API(logger);

	kill_kernel();

	exit(0);
}

void init_kernel() {
	system("clear");
	printf("<LFS Kernel>\n\n");
	srandom((int)time(NULL));
	load_logger();
	log_info(logger, "Iniciando Kernel");

	MP = get_multiprogramming_degree();

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
	sem_init(&MUTEX_READS, 0, 1);
	sem_init(&MUTEX_WRITES, 0, 1);
	sem_init(&MUTEX_TOTALOPS, 0, 1);
	init_memory();
}

void kill_kernel() {
	log_info(logger, "Terminando Kernel");
	log_info(logger, "------------------------------------------------------------");
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
			journal();
			break;

		case QUERY_ADD:
			add_memory_to_cons_type(atoi(args[2]), getConsistencyType(args[4]));
			sprintf(log_msg, " >> Recibi un ADD MEMORY %s TO %s", args[2], args[4]);
			if(getConsistencyType(args[4]) == CONS_SHC) {
				update_shc();
				display_memories();
			}
			break;

		case QUERY_RUN:
			if(!read_lql_file(args[1]))
				return queryError();
			sprintf(log_msg, " >> Recibi un RUN %s", args[1]);
			break;

		case QUERY_METRICS:
			sprintf(log_msg, " >> Recibi un METRICS");
			printf("\033[A");
			printf_metrics();
			printf("\n");
			break;

		default:
			return queryError(logger);
	}

	printf("\33[2K");

	log_info(logger, log_msg);

	if(isQuery) {
		t_query *newQuery = query_create(queryType, args);
		t_list *queryList = list_create();
		list_add(queryList, (void*)newQuery);
		t_process *newProcess = process_create(processNumber, queryList);
		processNumber++;

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

	t_process *newProcess = process_create(processNumber, fileQuerys);
	processNumber++;
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

t_process *ready_to_exec(int processor) {
	if(processor >= MP)
		return NULL;
	sem_wait(&PROC_PEND_READY);
	sem_wait(&MUTEX_READY);
	t_process *p = (t_process *)queue_pop(ready);
	sem_post(&MUTEX_READY);
	char msg[50];
	sprintf(msg, " >> Proceso %d ejecutando en procesador %d", p->pid, processor);
	log_info(logger, msg);
	return p;
}

void *processor_execute(void *p) {
	int processor = (int)p;
	time_t startTime, endTime;
	if(processor >= MP)
		return NULL;
	char msg[50];
	t_process *exec;
	while(true) {
		exec = ready_to_exec(processor);

		for(int i = 0; i < get_quantum(); i++) {
			if(process_finished(exec))
				break;
			t_query *nextQuery = process_next_query(exec);

			if(getQueryType(nextQuery->args[0]) == QUERY_ERROR || validateQuerySyntax(nextQuery->args, nextQuery->queryType) == 0) {
				sprintf(msg, " >> Error al ejecutar el proceso %d en la linea %d", exec->pid, exec->pc);
				log_error(logger, msg);
				exec->pc = process_length(exec);
				break;
			} else {
				time(&startTime);
				execute_query(nextQuery);
				sleep(get_execution_delay() / 1000);
				time(&endTime);

				if(nextQuery->queryType == QUERY_SELECT) metrics_new_select(startTime, endTime);
				if(nextQuery->queryType == QUERY_INSERT) metrics_new_insert(startTime, endTime);
			}
		}

		if(process_finished(exec)) {
			sprintf(msg, " >> Terminando proceso %d", exec->pid);
			log_info(logger, msg);
			process_destroy(exec);
			sem_post(&MAX_PROC_READY);
		} else {
			add_process_to_ready(exec);
		}
	}
	return NULL;
}

void execute_query(t_query *query) {
	switch(query->queryType) {
		case QUERY_SELECT:
			log_info(logger, " >> Ejecutando un SELECT %s %s", query->args[1], query->args[2]);
			qSelect(query->args, logger);
			break;
		case QUERY_INSERT:
			log_info(logger, " >> Ejecutando un INSERT %s %s \"%s\"", query->args[1], query->args[2], query->args[3]);
			qInsert(query->args, logger);
			break;
		case QUERY_CREATE:
			log_info(logger, " >> Ejecutando un CREATE %s %s %s %s", query->args[1], query->args[2], query->args[3], query->args[4]);
			qCreate(query->args, logger);
			break;
		case QUERY_DESCRIBE:
			log_info(logger, " >> Ejecutando un DESCRIBE %s", query->args[1]);
			qDescribe(query->args, logger);
			break;
		case QUERY_DROP:
			log_info(logger, " >> Ejecutando un DROP %s", query->args[1]);
			qDrop(query->args, logger);
			break;
		default: break;
	}
}

void init_memory() {
	request_memory_pool(0);
	char **args = parseQuery("DESCRIBE");
	qDescribe(args, logger);
//	t_memory *scMem = get_sc_memory();
//	printf("mid: %d", scMem->mid);
}

int connect_to_memory(char *IP, int PORT) {
	int memSocket = connectToServer(IP, PORT);
	if(memSocket == -1) {
		log_error(logger, "No se pudo conectar a Memoria.");
		exit(-1);
	}
	return memSocket;
}

void request_memory_pool(int memSocket) {
//	enviar REQUEST_GOSSIP
//	recibir RESPONSE_SUCCESS cant_memorias sizeip ip size port port n veces

	//Mock
	t_memory *mem = memory_create(memoryNumber, get_memory_ip(), get_memory_port());
	memoryNumber++;
	memory_add_cons_type(mem, CONS_SC);
//	memory_add_cons_type(mem, CONS_SHC);
	memory_add_cons_type(mem, CONS_EC);
	sem_wait(&MUTEX_MEMORIES);
	list_add(memories, (void*) mem);
	sem_post(&MUTEX_MEMORIES);
}

void display_memories() {
	printf("MID	IP		PORT	SC-SHC-EC\n");

	void display_memory(void *memory) {
		printf("%d	%s	%s	%d-%d-%d\n", ((t_memory*)memory)->mid, ((t_memory*)memory)->ip, ((t_memory*)memory)->port, ((t_memory*)memory)->consTypes[0], ((t_memory*)memory)->consTypes[1], ((t_memory*)memory)->consTypes[2]);
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

t_list *get_sc_memories() {
	bool memory_is_sc(void *mem) {
		return memory_is_cons_type((t_memory*)mem, CONS_SC);
	}
	sem_wait(&MUTEX_MEMORIES);
	t_list *sc_mem = list_filter(memories, memory_is_sc);
	sem_post(&MUTEX_MEMORIES);
	return sc_mem;
}

t_memory *get_sc_memory_for_table(t_table* t) {
	t_list *sc_mem = get_sc_memories();
	bool memByID(void *mem) {
		return ((t_memory*)mem)->mid == (int)list_get(t->memories, 0);
	}
	return (t_memory*)list_find(sc_mem, memByID);
}

t_memory *get_random_sc_memory() {
	t_list *sc_mem = get_sc_memories();
	return (t_memory*)list_get(sc_mem, random() % list_size(sc_mem));
}

t_list *get_shc_memories() {
	bool memory_is_shc(void *mem) {
		return memory_is_cons_type((t_memory*)mem, CONS_SHC);
	}
	sem_wait(&MUTEX_MEMORIES);
	t_list *shc_mem = list_filter(memories, memory_is_shc);
	sem_post(&MUTEX_MEMORIES);
	return shc_mem;
}

t_memory *get_shc_memory_for_table(t_table *t, uint16_t key) {
	t_list *shc_mem = get_shc_memories();
	return (t_memory*)list_get(shc_mem, key % list_size(shc_mem));
}

t_list *get_ec_memories() {
	bool memory_is_ec(void *mem) {
		return memory_is_cons_type((t_memory*)mem, CONS_EC);
	}
	sem_wait(&MUTEX_MEMORIES);
	t_list *ec_mem = list_filter(memories, memory_is_ec);
	sem_post(&MUTEX_MEMORIES);
	return ec_mem;
}

t_memory *get_ec_memory() {
	t_list *ec_mem = get_ec_memories();
	int index = random() % list_size(ec_mem);
	return (t_memory*)list_get(ec_mem, index);
}

t_memory *get_memory_for_query(t_table *t, uint16_t key) {
	t_memory *m = NULL;
	switch(t->consType) {
		case CONS_SC: m = get_sc_memory_for_table(t); break;
		case CONS_SHC: m = get_shc_memory_for_table(t, key); break;
		case CONS_EC: m = get_ec_memory(); break;
		default: break;
	}
	return m;
}

t_memory *get_any_memory() {
	sem_wait(&MUTEX_MEMORIES);
	t_memory *m = list_get(memories, random() % list_size(memories));
	sem_post(&MUTEX_MEMORIES);
	return m;
}

void update_shc() {
	bool isSHC(void *t) {
		return ((t_table*)t)->consType == CONS_SHC;
	}
	void updateMemories(void *t) {
		add_memories_to_table((t_table*)t);
	}
	void journalMem(void *m) {
		qJournal((t_memory*)m, logger); //TODO implementar journal en memoria
	}
	sem_wait(&MUTEX_TABLES);
	t_list *shcTables = list_filter(tables, isSHC);
	list_iterate(shcTables, updateMemories);
	sem_post(&MUTEX_TABLES);
	sem_wait(&MUTEX_MEMORIES);
	t_list *shc_mem = get_shc_memories();
	list_iterate(shc_mem, journalMem);
	sem_post(&MUTEX_MEMORIES);
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
		add_table(t);
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

void add_memories_to_table(t_table *t) {
	void addMemoryToTable(void *mem) {
		table_add_memory_by_id(t, ((t_memory*)mem)->mid);
	}
	switch(t->consType) {
		case CONS_SC:
			if(list_size(get_sc_memories()) > 0) {
				table_add_memory_by_id(t, get_random_sc_memory()->mid);
			}
			break;
		case CONS_SHC:
			list_iterate(get_shc_memories(), addMemoryToTable);
			break;
		default:
			break;
	}
}

void journal(){
	void journalMem(void *m) {
			qJournal((t_memory*)m, logger);
		}
	sem_wait(&MUTEX_MEMORIES);
		list_iterate(memories,journalMem);
	sem_post(&MUTEX_MEMORIES);
}

void *print_metrics() {
	printf("\nR : RL/30s = 0 : 0.00s\nW : WL/30s = 0 : 0.00s\nML = \n\n");

	while(true) {
		sleep(30);
		sem_wait(&MUTEX_READS);
		sem_wait(&MUTEX_WRITES);
		printf_metrics();
		reads = 0;
		readsTime = 0.0f;
		writes = 0;
		writesTime = 0.0f;
		sem_post(&MUTEX_READS);
		sem_post(&MUTEX_WRITES);

	}

	return NULL;
}

void printf_metrics() {
	void print_m_load(void *mem) {
		t_memory *m = (t_memory*)mem;
		int mLoad = (m->totalOperations * 100) / totalOperations;
		printf("%d : %d%% ", m->mid, mLoad);
	}

	float readLatency = 0.0f, writeLatency = 0.0f;

	if(reads != 0) {
		readLatency = readsTime/reads;
	} else {
		readLatency = 0.0f;
	}
	if(writes != 0) {
		writeLatency = writesTime/writes;
	} else {
		writeLatency = 0.0f;
	}
	printf("%c7", 27);
	printf("\033[A\033[A\033[A\033[A");
	printf("\33[2K\rR : RL/30s = %d : %.2fs\n\33[2KW : WL/30s = %d : %.2fs\n\33[2KML = ", reads, readLatency, writes, writeLatency);
	if(totalOperations != 0) {
		sem_wait(&MUTEX_MEMORIES);
		list_iterate(memories, print_m_load);
		sem_post(&MUTEX_MEMORIES);
	}
	printf("%c8", 27);
}

void metrics_new_select(int start, int end) {
	sem_wait(&MUTEX_READS);
	reads++;
	readsTime += (float)difftime(end, start);
	sem_post(&MUTEX_READS);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

void metrics_new_insert(int start, int end) {
	sem_wait(&MUTEX_WRITES);
	writes++;
	writesTime += (float)difftime(end, start);
	sem_post(&MUTEX_WRITES);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

char *get_memory_ip() {
	load_config();
	char *ip = strdup(config_get_string_value(config, "MEM_IP"));
	config_destroy(config);
	return ip;
}

char *get_memory_port() {
	load_config();
	char *port = strdup(config_get_string_value(config, "MEM_PORT"));
	config_destroy(config);
	return port;
}

int get_quantum() {
	load_config();
	int q = config_get_int_value(config, "QUANTUM");
	config_destroy(config);
	return q;
}

int get_multiprogramming_degree() {
	load_config();
	int md = config_get_int_value(config, "MULT_DEGREE");
	config_destroy(config);
	return md;
}

int get_metadata_refresh_rate() {
	load_config();
	int mdr = config_get_int_value(config, "MD_REFRESH_RATE");
	config_destroy(config);
	return mdr;
}

int get_execution_delay() {
	load_config();
	int ed = config_get_int_value(config, "EXEC_DELAY");
	config_destroy(config);
	return ed;
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
