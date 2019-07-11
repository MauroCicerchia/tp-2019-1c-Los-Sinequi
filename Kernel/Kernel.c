#include"Kernel.h"

//CONFIG
int QUANTUM, MP_DEGREE, MD_REFRESH, EXEC_DELAY;
char *MEM_IP, *MEM_PORT;

int server, processNumber = 0, memoryNumber = 0, MP, reads = 0, writes = 0, totalOperations = 0, exitFlag = 0, fd, wd, lastMP;
uint16_t readsTime = 0, writesTime = 0;
t_list *memories, *tables, *threadsExec, *processorsExitFlags;
t_config *config;
t_queue *new, *ready;
sem_t MUTEX_NEW, MUTEX_READY, MUTEX_MEMORIES, MUTEX_TABLES, PROC_PEND_NEW, MAX_PROC_READY, PROC_PEND_READY, MUTEX_READS, MUTEX_WRITES, MUTEX_TOTALOPS, MUTEX_CONFIG;
t_log *logger;
char *configPath;

int main(int argc, char **argv) {
	init_kernel();

	display_memories();

	pthread_t threadNewReady, threadMetrics, threadConfig;

	pthread_create(&threadNewReady, NULL, new_to_ready, NULL); //TODO agrega joins, pelotudo!
	pthread_create(&threadMetrics, NULL, metrics, NULL);
//	pthread_create(&threadGossip, NULL, gossip, NULL);
	for(int i = 0; i < get_multiprocessing_degree(); i++) {
		createProcessorThread();
	}
	pthread_create(&threadConfig, NULL, threadConfigModify, NULL);

	usleep(10000);

	start_API(logger);

	kill_kernel();

	exit(0);
}

void init_kernel() {
	system("clear");
	printf("<LFS Kernel>\n\n");
	srandom((int)time(NULL));
	load_logger();
	log_info(logger, " >> INICIO KERNEL >>");

	configPath = string_duplicate("../.config");

	fd = inotify_init(); //arranco monitoreo en el archivo de config
	wd = inotify_add_watch(fd, configPath, IN_MODIFY);

	sem_init(&MUTEX_CONFIG, 0, 1);

	sem_wait(&MUTEX_CONFIG);
	load_config();
	MEM_IP = string_duplicate(config_get_string_value(config, "MEM_IP"));
	MEM_PORT = string_duplicate(config_get_string_value(config, "MEM_PORT"));
	QUANTUM = config_get_int_value(config, "QUANTUM");
	MP_DEGREE = config_get_int_value(config, "MULT_DEGREE");
	MD_REFRESH = config_get_int_value(config, "MD_REFRESH_RATE");
	EXEC_DELAY = config_get_int_value(config, "EXEC_DELAY");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);

	lastMP = get_multiprocessing_degree();

	threadsExec = list_create();
	processorsExitFlags = list_create();
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
//	sem_init(&MAX_PROC_READY, 0, get_multiprocessing_degree());
	sem_init(&MUTEX_READS, 0, 1);
	sem_init(&MUTEX_WRITES, 0, 1);
	sem_init(&MUTEX_TOTALOPS, 0, 1);
	init_memory();
}

void kill_kernel() {
	exitFlag = 1;
	printf("\nTerminando kernel, por favor espere...\n");
	log_info(logger, " >> Terminando kernel, por favor espere... >>");
	queue_destroy_and_destroy_elements(new, process_destroy);
	queue_destroy_and_destroy_elements(ready, process_destroy);
	list_destroy_and_destroy_elements(memories, memory_destroy);
	for(int i = 0; i < list_size(threadsExec); i++) {
		deleteProcessorThread();
	}
	sem_destroy(&MUTEX_NEW); //TODO revisar sem faltantes
	sem_destroy(&MUTEX_READY);
	sem_destroy(&MUTEX_CONFIG);
	sem_destroy(&PROC_PEND_NEW);
	sem_destroy(&PROC_PEND_READY);
//	sem_destroy(&MAX_PROC_READY);
	free(configPath);
	log_info(logger, " << FIN KERNEL <<");
	log_info(logger, "------------------------------------------------------------------------------------------------------------");
	log_destroy(logger);
}

e_query processQuery(char *query) {
	e_query queryType;

	t_list *args = validate_query_and_return_args(query);
	if(args == NULL)
		return queryError();

	queryType = getQueryType(list_get(args, 0));

	int isQuery = 0;
	switch(queryType) {
		case QUERY_SELECT:
			isQuery = 1;
			log_info(logger, " >> Recibi un SELECT %s %s", (char*)list_get(args, 1), (char*)list_get(args, 2));
			break;

		case QUERY_INSERT:
			isQuery = 1;
			log_info(logger, " >> Recibi un INSERT %s %s %s", (char*)list_get(args, 1), (char*)list_get(args, 2), (char*)list_get(args, 3));
			break;

		case QUERY_CREATE:
			isQuery = 1;
			log_info(logger, " >> Recibi un CREATE %s %s %s %s", (char*)list_get(args, 1), (char*)list_get(args, 2), (char*)list_get(args, 3), (char*)list_get(args, 4));
			break;

		case QUERY_DESCRIBE:
			isQuery = 1;
			log_info(logger, " >> Recibi un DESCRIBE %s", (char*)list_get(args, 1));
			break;

		case QUERY_DROP:
			isQuery = 1;
			log_info(logger, " >> Recibi un DROP %s", (char*)list_get(args, 1));
			break;

		case QUERY_JOURNAL:
			log_info(logger, " >> Recibi un JOURNAL");
			journal();
			break;

		case QUERY_ADD:
			add_memory_to_cons_type(atoi((char*)list_get(args, 2)), getConsistencyType((char*)list_get(args, 4)));
			log_info(logger, " >> Recibi un ADD MEMORY %s TO %s", (char*)list_get(args, 2), (char*)list_get(args, 4));
			if(getConsistencyType((char*)list_get(args, 4)) == CONS_SHC) {
				update_shc();
			}
			printf("\033[A");
			update_screen();
			break;

		case QUERY_RUN:
			if(!read_lql_file((char*)list_get(args, 1)))
				return queryError();
			log_info(logger, " >> Recibi un RUN %s", (char*)list_get(args, 1));
			break;

		case QUERY_METRICS:
			log_info(logger, " >> Recibi un METRICS");
			printf("\033[A");
			update_screen();
			log_metrics();
			break;

		default:
			return queryError(logger);
	}

	printf("\33[2K");

	if(isQuery) {
		t_query *newQuery = query_create(queryType, args);
		t_list *queryList = list_create();
		list_add(queryList, (void*)newQuery);
		t_process *newProcess = process_create(processNumber, queryList);
		processNumber++;

		add_process_to_new(newProcess);
	} else {
		list_destroy_and_destroy_elements(args, string_destroy);
	}
	return queryType;
}

int read_lql_file(char *path) {
	char buffer[500];
	FILE *lql = fopen(path, "rt");
	if(lql == NULL) {
		log_error(logger, " >> No se ha podido abrir el archivo %s", path);
		return 0;
	}

	t_list *fileQuerys = list_create();

	while(fgets(buffer, sizeof(buffer), lql)) {
		log_info(logger,buffer);
		t_list *args = parseQuery(buffer);
		e_query queryType = getQueryType(list_get(args, 0));
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
	sem_wait(&MUTEX_NEW);
	queue_push(new, (void*) process);
	log_info(logger, " >> Proceso %d agregado a la cola de NEW", process->pid);
	sem_post(&MUTEX_NEW);
	sem_post(&PROC_PEND_NEW);

}

void *new_to_ready() {
	while(!exitFlag) {
//		sem_wait(&MAX_PROC_READY);
		sem_wait(&PROC_PEND_NEW);
		sem_wait(&MUTEX_NEW);
		void *p = queue_pop(new);
		log_info(logger, " >> Proceso %d agregado a la cola de READY", ((t_process*)p)->pid);
		sem_post(&MUTEX_NEW);
		add_process_to_ready(p);
	}
	return NULL;
}

void add_process_to_ready(t_process *process) {
	sem_wait(&MUTEX_READY);
	queue_push(ready, process);
	sem_post(&MUTEX_READY);
	sem_post(&PROC_PEND_READY);
}

t_process *ready_to_exec(int processor) {
	sem_wait(&PROC_PEND_READY);
	sem_wait(&MUTEX_READY);
	t_process *p = (t_process *)queue_pop(ready);
	log_info(logger, " >> Proceso %d ejecutando en procesador %d", p->pid, processor);
	sem_post(&MUTEX_READY);
	return p;
}

void createProcessorThread() {
	pthread_t thread;
	list_add(threadsExec, &thread);
	int *flag = (int*)malloc(sizeof(int));
	*flag = 0;
	list_add(processorsExitFlags, flag);
	pthread_create(&thread, NULL, processor_execute, (void*)(list_size(threadsExec) - 1));
	log_info(logger, " >> Procesador %d creado.", list_size(threadsExec) - 1);
}

void deleteProcessorThread() {
	int index = list_size(threadsExec) - 1;
	int *flag = (int*)list_get(processorsExitFlags, index);
	*flag = 1;
	pthread_t *thread = list_remove(threadsExec, index);
	pthread_join(*thread, NULL);
	log_info(logger, " >> Procesador %d eliminado.", index);
}

void *processor_execute(void *p) {
	int processor = (int)p;
	uint16_t startTime, endTime;
	t_process *exec;
	while(!exitFlag && !*(int*)list_get(processorsExitFlags, processor)) {
		log_info(logger, "Flag de p%d: %d", processor, *(int*)list_get(processorsExitFlags, processor));
		exec = ready_to_exec(processor);

		for(int i = 0; i < get_quantum(); i++) {
			if(process_finished(exec))
				break;
			t_query *nextQuery = process_next_query(exec);

			if(getQueryType(list_get(nextQuery->args, 0)) == QUERY_ERROR || validateQuerySyntax(nextQuery->args) == 0) {
				log_error(logger, " >> Error al ejecutar el proceso %d en la linea %d", exec->pid, exec->pc);
				exec->pc = process_length(exec);
				break;
			} else {
				startTime = getCurrentTime();
				int status = execute_query(nextQuery);
				usleep(get_execution_delay() * 1000);
				endTime = getCurrentTime();

				if(status && nextQuery->queryType == QUERY_SELECT) metrics_new_select(startTime, endTime);
				if(status && nextQuery->queryType == QUERY_INSERT) metrics_new_insert(startTime, endTime);
			}
		}

		if(process_finished(exec)) {
			log_info(logger, " >> Terminando proceso %d", exec->pid);
			process_destroy(exec);
//			sem_post(&MAX_PROC_READY);
		} else {
			add_process_to_ready(exec);
		}
	}
	list_remove_and_destroy_element(processorsExitFlags, processor, free);
	return NULL;
}

int execute_query(t_query *query) {
	switch(query->queryType) {
		case QUERY_SELECT:
			log_info(logger, " >> Ejecutando un SELECT %s %s", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2));
//			return qSelect(list_get(query->args, 1), strtol(list_get(query->args, 2),NULL, 10), logger);
			return 1;
			break;
		case QUERY_INSERT:
			log_info(logger, " >> Ejecutando un INSERT %s %s \"%s\"", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2), (char*)list_get(query->args, 3));
//			return qInsert(list_get(query->args, 1), strtol(list_get(query->args, 2),NULL, 10), list_get(query->args, 3), logger);
			return 1;
			break;
		case QUERY_CREATE:
			log_info(logger, " >> Ejecutando un CREATE %s %s %s %s", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2), (char*)list_get(query->args, 3), (char*)list_get(query->args, 4));
//			return qCreate(list_get(query->args, 1), list_get(query->args, 2), list_get(query->args, 3), list_get(query->args, 4), logger);
			return 1;
			break;
		case QUERY_DESCRIBE:
			if(list_get(query->args, 1) != NULL) {
				log_info(logger, " >> Ejecutando un DESCRIBE %s", (char*)list_get(query->args, 1));
//				return qDescribe(list_get(query->args, 1), logger);
				return 1;
			} else {
				log_info(logger, " >> Ejecutando un DESCRIBE");
//				return qDescribe(NULL, logger);
				return 1;
			}
			break;
		case QUERY_DROP:
			log_info(logger, " >> Ejecutando un DROP %s", (char*)list_get(query->args, 1));
//			return qDrop(list_get(query->args, 1), logger);
			return 1;
			break;
		default: return 0; break;
	}
}

void init_memory() {
	char *ip = get_memory_ip();
	char *port = get_memory_port();
	log_info(logger, " >> Conectando a primer memoria.");
	int memSocket = connect_to_memory(ip, port);
	log_info(logger, " >> Solicitando memorias disponibles.");
	request_memory_pool(memSocket);
	log_info(logger, " >> Memorias obtenidas correctamente.");
	log_info(logger, " >> Realizando Describe inicial.");
	qDescribe(NULL, logger); 
	log_info(logger, " >> Tablas obtenidas correctamente.");
	close(memSocket);
	free(ip);
	free(port);
}

int connect_to_memory(char *IP, int PORT) {
	int memSocket = connectToServer(IP, PORT);
	if(memSocket <= -1) {
		log_error(logger, " >> No se pudo conectar a Memoria.");
		exit(-1);
	}
	return memSocket;
}

void request_memory_pool(int memSocket) {
	send_req_code(memSocket, REQUEST_GOSSIP);
	send_int(memSocket, 0);
	e_response_code response = recv_res_code(memSocket);
	if(response == RESPONSE_SUCCESS) {
		int q = recv_int(memSocket);
		int i;
		char *ip, *port;
		for(i = 0; i < q; i++) {
			ip = recv_str(memSocket);
			port = recv_str(memSocket);
			t_memory *mem = memory_create(memoryNumber, ip, port);
			memoryNumber++;
			sem_wait(&MUTEX_MEMORIES);
			list_add(memories, (void*) mem);
			sem_post(&MUTEX_MEMORIES);
		}
	}
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
	t_memory *m = (t_memory*)list_find(sc_mem, memByID);
	list_destroy(sc_mem);
	return m;
}

t_memory *get_random_sc_memory() {
	t_memory *m = NULL;
	t_list *sc_mem = get_sc_memories();
	if(list_size(sc_mem) > 0)
		m = (t_memory*)list_get(sc_mem, random() % list_size(sc_mem));
	list_destroy(sc_mem);
	return m;
}

t_list *get_shc_memories() {
	bool memory_is_shc(void *mem) {
		return memory_is_cons_type((t_memory*)mem, CONS_SHC);
	}
	t_list *shc_mem = list_filter(memories, memory_is_shc);
	return shc_mem;
}

t_memory *get_shc_memory_for_table(t_table *t, uint16_t key) {
	t_list *shc_mem = get_shc_memories();
	if(list_size(shc_mem) > 0) {
		t_memory *mem = list_get(shc_mem, key % list_size(shc_mem));
		list_destroy(shc_mem);
		return mem;
	}
	return NULL;
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
	t_memory *mem;
	t_list *ec_mem = get_ec_memories();
	if(list_size(ec_mem) > 0) {
		int index = random() % list_size(ec_mem);
		mem = list_get(ec_mem, index);
	} else {
		mem = NULL;
	}
	list_destroy(ec_mem);
	return mem;
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
		qJournal((t_memory*)m, logger);
	}
	sem_wait(&MUTEX_TABLES);
	t_list *shcTables = list_filter(tables, isSHC);
	list_iterate(shcTables, updateMemories);
	sem_post(&MUTEX_TABLES);
	sem_wait(&MUTEX_MEMORIES);
	t_list *shc_mem = get_shc_memories();
	sem_post(&MUTEX_MEMORIES);

	if(list_size(shc_mem) > 1)
		list_iterate(shc_mem, journalMem);
	list_destroy(shcTables);
	list_destroy(shc_mem);
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
	t_list *mems;
	switch(t->consType) {
		case CONS_SC:
			mems = get_sc_memories();
			if(list_size(mems) > 0) {
				table_add_memory_by_id(t, get_random_sc_memory()->mid);
			}
			list_destroy(mems);
			break;
		case CONS_SHC:
			mems = get_shc_memories();
			list_clean(t->memories);
			list_iterate(mems, addMemoryToTable);
			printf("%d", list_size(t->memories));
			list_destroy(mems);
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

void *metrics() {
	printf("\nR : RL/30s = 0 : 0ms\nW : WL/30s = 0 : 0ms\nML = \n\n");

	while(!exitFlag) {
		usleep(30 * 1000 * 1000);
		sem_wait(&MUTEX_READS);
		sem_wait(&MUTEX_WRITES);
		log_metrics();
		reads = 0;
		readsTime = 0;
		writes = 0;
		writesTime = 0;
		sem_post(&MUTEX_READS);
		sem_post(&MUTEX_WRITES);

	}

	return NULL;
}

void log_metrics() {
	int readLatency = 0, writeLatency = 0;
	if(reads != 0) {
		readLatency = readsTime/reads;
	}
	if(writes != 0) {
		writeLatency = writesTime/writes;
	}

	log_info(logger, " >> Metricas >>");
	log_info(logger, "R : RL/30s = %d : %dms", reads, readLatency);
	log_info(logger, "W : WL/30s = %d : %dms", writes, writeLatency);
	log_info(logger, " << Fin Metricas <<");
}

void update_screen() {
	void print_m_load(void *mem) {
		t_memory *m = (t_memory*)mem;
		int mLoad = (m->totalOperations * 100) / totalOperations;
		printf("%d : %d%% ", m->mid, mLoad);
	}

	int readLatency = 0, writeLatency = 0;

	if(reads != 0) {
		readLatency = readsTime/reads;
	}
	if(writes != 0) {
		writeLatency = writesTime/writes;
	}

	system("clear");
	printf("<LFS Kernel>\n\n");
	display_memories();

	printf("\nR : RL/30s = %d : %dms\nW : WL/30s = %d : %dms\nML = ", reads, readLatency, writes, writeLatency);
	if(totalOperations != 0) {
		sem_wait(&MUTEX_MEMORIES);
		list_iterate(memories, print_m_load);
		sem_post(&MUTEX_MEMORIES);
	}
	printf("\n\n\n");
}

void metrics_new_select(int start, int end) {
	sem_wait(&MUTEX_READS);
	reads++;
	readsTime += end - start;
	sem_post(&MUTEX_READS);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

void metrics_new_insert(int start, int end) {
	sem_wait(&MUTEX_WRITES);
	writes++;
	writesTime += end - start;
	sem_post(&MUTEX_WRITES);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

char *get_memory_ip() {
	sem_wait(&MUTEX_CONFIG);
	char *ip = string_duplicate(MEM_IP);
	sem_post(&MUTEX_CONFIG);
	return ip;
}

char *get_memory_port() {
	sem_wait(&MUTEX_CONFIG);
	char *port = string_duplicate(MEM_PORT);
	sem_post(&MUTEX_CONFIG);
	return port;
}

int get_quantum() {
	sem_wait(&MUTEX_CONFIG);
	int q = QUANTUM;
	sem_post(&MUTEX_CONFIG);
	return q;
}

int get_multiprocessing_degree() {
	sem_wait(&MUTEX_CONFIG);
	int mp = MP_DEGREE;
	sem_post(&MUTEX_CONFIG);
	return mp;
}

int get_metadata_refresh_rate() {
	sem_wait(&MUTEX_CONFIG);
	int mdr = MD_REFRESH;
	sem_post(&MUTEX_CONFIG);
	return mdr;
}

int get_execution_delay() {
	sem_wait(&MUTEX_CONFIG);
	int ed = EXEC_DELAY;
	sem_post(&MUTEX_CONFIG);
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
	config = config_create(configPath);
	if(config == NULL) {
		log_error(logger, " >> No se pudo abrir el archivo de configuracion");
		exit(-1);
	}
}

void *threadConfigModify()
{
	char *buff = NULL;
	while(1){
		buff = malloc(200);
		read(fd,buff,200);
		free(buff);
		log_info(logger, " >> Se podrujo un cambio en .config");
		log_info(logger, " >> Actualizando valores...");

		sem_wait(&MUTEX_CONFIG);
		free(MEM_IP);
		free(MEM_PORT);
		load_config();
		MEM_IP = string_duplicate(config_get_string_value(config, "MEM_IP"));
		MEM_PORT = string_duplicate(config_get_string_value(config, "MEM_PORT"));
		QUANTUM = config_get_int_value(config, "QUANTUM");
		MP_DEGREE = config_get_int_value(config, "MULT_DEGREE");
		MD_REFRESH = config_get_int_value(config, "MD_REFRESH_RATE");
		EXEC_DELAY = config_get_int_value(config, "EXEC_DELAY");
		config_destroy(config);
		sem_post(&MUTEX_CONFIG);

		if(lastMP != get_multiprocessing_degree()) {
			if(lastMP < get_multiprocessing_degree()) { //Procesador/es nuevo/s
				int q = get_multiprocessing_degree() - lastMP;
				if(q == 1) {
					log_info(logger, " >> Agregando 1 nuevo procesador.");
				} else {
					log_info(logger, " >> Agregando %d nuevos procesadores.", q);
				}
				for(int i = 0; i < q; i++) {
					createProcessorThread();
				}
			} else if(lastMP > get_multiprocessing_degree()) { //Procesadores a eliminar
				int q = lastMP - get_multiprocessing_degree();
				if(q == 1) {
					log_info(logger, " >> Eliminando 1 procesador.");
				} else {
					log_info(logger, " >> Eliminando %d procesadores.", q);
				}
				for(int i = 0; i < q; i++) {
					deleteProcessorThread();
				}
			}
			lastMP = get_multiprocessing_degree();
		}
	}
	return NULL;
}

uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}
