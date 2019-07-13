#include"Kernel.h"

int server, processNumber = 0, memoryNumber = 0, MP, totalOperations = 0, exitFlag = 0;
uint16_t readsTime = 0, writesTime = 0;
t_list *memories, *tables, *reads, *writes;
t_config *config;
t_queue *new, *ready;
sem_t MUTEX_NEW, MUTEX_READY, MUTEX_MEMORIES, MUTEX_TABLES, MUTEX_CONFIG, PROC_PEND_NEW, MAX_PROC_READY, PROC_PEND_READY, MUTEX_READS, MUTEX_WRITES, MUTEX_TOTALOPS;
t_log *logger;
pthread_t threadNewReady, threadMetrics, threadGossip, threadRefreshMetadata;

int main(int argc, char **argv) {

	init_kernel();

	pthread_t threadsExec[MP];

	pthread_create(&threadNewReady, NULL, new_to_ready, NULL);
	pthread_detach(threadNewReady);
	pthread_create(&threadMetrics, NULL, metrics, NULL);
	pthread_create(&threadRefreshMetadata, NULL, refreshMetadata, NULL);
	pthread_create(&threadGossip, NULL, gossip, NULL);
	for(int i = 0; i < MP; i++) {
		pthread_create(&threadsExec[i], NULL, processor_execute, (void*)i);
		pthread_detach(threadsExec[i]);
	}

	usleep(10000);

	start_API(logger);

	exitFlag = 1;

	printf("\n >> Terminando kernel, por favor espere...\n");
	log_info(logger, " >> Terminando kernel, por favor espere... >>");

	pthread_join(threadMetrics, NULL);
	pthread_join(threadRefreshMetadata, NULL);
	pthread_join(threadGossip, NULL);

	kill_kernel();

	exit(0);
}

void init_kernel() {
	system("clear");
	printf("<LFS Kernel>\n\n");
	srandom((int)time(NULL));
	load_logger();
	log_info(logger, " >> INICIO KERNEL >>");

	sem_init(&MUTEX_CONFIG, 0, 1);

	MP = get_multiprogramming_degree();

	new = queue_create();
	ready = queue_create();
	memories = list_create();
	tables = list_create();
	reads = list_create();
	writes = list_create();
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
	log_info(logger, " << FIN KERNEL <<");
	log_info(logger, "------------------------------------------------------------------------------------------------------------");
	log_destroy(logger);
	queue_destroy_and_destroy_elements(new, process_destroy);
	queue_destroy_and_destroy_elements(ready, process_destroy);
	list_destroy_and_destroy_elements(memories, memory_destroy);
	list_destroy_and_destroy_elements(reads, operation_destroy);
	list_destroy_and_destroy_elements(writes, operation_destroy);
	sem_destroy(&MUTEX_NEW);
	sem_destroy(&MUTEX_READY);
	sem_destroy(&MUTEX_CONFIG);
	sem_destroy(&PROC_PEND_NEW);
	sem_destroy(&PROC_PEND_READY);
	sem_destroy(&MAX_PROC_READY);
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
			log_info(logger, " >> Recibi un ADD MEMORY %s TO %s", (char*)list_get(args, 2), (char*)list_get(args, 4));
			add_memory_to_cons_type(atoi((char*)list_get(args, 2)), getConsistencyType((char*)list_get(args, 4)));
			if(getConsistencyType((char*)list_get(args, 4)) == CONS_SHC) {
				update_shc();
			}
			if(getConsistencyType((char*)list_get(args, 4)) == CONS_SC) {
				update_sc();
			}
			printf("\033[A");
			update_screen();
			break;

		case QUERY_RUN:
			log_info(logger, " >> Recibi un RUN %s", (char*)list_get(args, 1));
			if(!read_lql_file((char*)list_get(args, 1)))
				return queryError();
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
		sem_wait(&MAX_PROC_READY);
		sem_wait(&PROC_PEND_NEW);
		sem_wait(&MUTEX_NEW);
		void *p = queue_pop(new);
		sem_post(&MUTEX_NEW);
		add_process_to_ready(p);
		log_info(logger, " >> Proceso %d agregado a la cola de READY", ((t_process*)p)->pid);
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
	if(processor >= MP)
		return NULL;
	sem_wait(&PROC_PEND_READY);
	sem_wait(&MUTEX_READY);
	t_process *p = (t_process *)queue_pop(ready);
	sem_post(&MUTEX_READY);
	log_info(logger, " >> Proceso %d ejecutando en procesador %d", p->pid, processor);
	return p;
}

void *processor_execute(void *p) {
	int processor = (int)p;
	uint64_t startTime, endTime;
	if(processor >= MP)
		return NULL;
	t_process *exec;
	while(!exitFlag) {
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

				if(status == -1) {
					exec->pc = process_length(exec);
					log_error(logger, " >> Abortando proceso %d", exec->pid);
					break;
				}

				if(status == 1 && nextQuery->queryType == QUERY_SELECT) metrics_new_select(startTime, endTime);
				if(status == 1 && nextQuery->queryType == QUERY_INSERT) metrics_new_insert(startTime, endTime);
			}
		}

		if(process_finished(exec)) {
			log_info(logger, " >> Terminando proceso %d", exec->pid);
			process_destroy(exec);
			sem_post(&MAX_PROC_READY);
		} else {
			add_process_to_ready(exec);
		}
	}
	return NULL;
}

int execute_query(t_query *query) {
	switch(query->queryType) {
		case QUERY_SELECT:
			log_info(logger, " >> Ejecutando un SELECT %s %s", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2));
			return qSelect(list_get(query->args, 1), strtol(list_get(query->args, 2),NULL, 10), logger);
			break;
		case QUERY_INSERT:
			log_info(logger, " >> Ejecutando un INSERT %s %s \"%s\"", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2), (char*)list_get(query->args, 3));
			return qInsert(list_get(query->args, 1), strtol(list_get(query->args, 2),NULL, 10), list_get(query->args, 3), logger);
			break;
		case QUERY_CREATE:
			log_info(logger, " >> Ejecutando un CREATE %s %s %s %s", (char*)list_get(query->args, 1), (char*)list_get(query->args, 2), (char*)list_get(query->args, 3), (char*)list_get(query->args, 4));
			return qCreate(list_get(query->args, 1), list_get(query->args, 2), list_get(query->args, 3), list_get(query->args, 4), logger);
			break;
		case QUERY_DESCRIBE:
			if(list_get(query->args, 1) != NULL) {
				log_info(logger, " >> Ejecutando un DESCRIBE %s", (char*)list_get(query->args, 1));
				return qDescribe(list_get(query->args, 1), logger);
			} else {
				log_info(logger, " >> Ejecutando un DESCRIBE");
				return qDescribe(NULL, logger);
			}
			break;
		case QUERY_DROP:
			log_info(logger, " >> Ejecutando un DROP %s", (char*)list_get(query->args, 1));
			return qDrop(list_get(query->args, 1), logger);
			break;
		default: return 0; break;
	}
}

void init_memory() {
	char *ip = get_memory_ip();
	char *port = get_memory_port();
	log_info(logger, " >> Conectando a primer memoria.");
	int memSocket;
	do {
		memSocket = connect_to_memory(ip, port);
		if(memSocket == -1) {
			log_warning(logger, "No se pudo conectar a la memoria principal para obtener las memorias disponibles. Intentando de nuevo en 10s.");
			usleep(10 *1000 * 1000);
		}
	} while(memSocket == -1);
	log_info(logger, " >> Solicitando memorias disponibles.");
	request_memory_pool(memSocket);
	log_info(logger, " >> Memorias obtenidas correctamente.");
	display_memories();
	log_info(logger, " >> Realizando Describe inicial.");
	qDescribe(NULL, logger); 
	log_info(logger, " >> Tablas obtenidas correctamente.");
	close(memSocket);
	free(ip);
	free(port);
}

int connect_to_memory(char *IP, char *PORT) {
	int memSocket = connectToServer(IP, PORT);
	if(memSocket <= -1) {
		log_error(logger, " >> No se pudo conectar a Memoria.");
		return -1;
	}
	return memSocket;
}

void request_memory_pool(int memSocket) {
	send_req_code(memSocket, REQUEST_GOSSIP);
	send_int(memSocket, 0);
	e_response_code response = recv_res_code(memSocket);
	if(response == RESPONSE_SUCCESS) {
		int q = recv_int(memSocket);
		int i, mem_number;
		char *ip, *port;
		for(i = 0; i < q; i++) {
			mem_number = recv_int(memSocket);
			ip = recv_str(memSocket);
			port = recv_str(memSocket);
			memory_search_create(mem_number, ip, port);
		}
	}
}

t_memory *memory_search_create(int mem_number, char *ip, char *port) {
	bool isMem(void *mem) {
		return ((t_memory*)mem)->mid == mem_number;
	}
	sem_wait(&MUTEX_MEMORIES);
	t_memory *m = (t_memory*)list_find(memories, isMem);
	if(m == NULL) {
		m = memory_create(mem_number, ip, port);
		list_add(memories, (void*) m);
	} else {
		free(ip);
		free(port);
	}
	sem_post(&MUTEX_MEMORIES);
	return m;
}

void *gossip() {
	while(!exitFlag) {

		int i = 0;
		for(i = 0; i < 10; i++) {
			usleep(get_gossip_delay() * 100);
			if(exitFlag) break;
		}

		if(!exitFlag) {
			char *ip = get_memory_ip();
			char *port = get_memory_port();
			log_info(logger, " >> Actualizando memorias.");
			int memSocket;
			do {
				memSocket = connect_to_memory(ip, port);
				if(memSocket == -1) {
					log_warning(logger, "No se pudo conectar a la memoria principal para actualizar. Intentando de nuevo en 10s.");
					usleep(10 * 1000 * 1000);
				}
			} while(memSocket == -1);
			request_memory_pool(memSocket);
			log_info(logger, " >> Memorias actualizadas.");
			free(ip);
			free(port);
		}
	}
	return NULL;
}

void display_memories() {
	printf("MID	IP		PORT	SC-SHC-EC\n");

	void display_memory(void *memory) {
		printf("%d	%s	%s	%d - %d - %d\n", ((t_memory*)memory)->mid, ((t_memory*)memory)->ip, ((t_memory*)memory)->port, ((t_memory*)memory)->consTypes[0], ((t_memory*)memory)->consTypes[1], ((t_memory*)memory)->consTypes[2]);
	}

	bool memCompare(void *m1, void *m2) {
		return ((t_memory*)m1)->mid < ((t_memory*)m2)->mid;
	}

	sem_wait(&MUTEX_MEMORIES);
	t_list *ordered_mems = list_sorted(memories, memCompare);
	list_iterate(memories, display_memory);
	list_destroy(ordered_mems);
	sem_post(&MUTEX_MEMORIES);

}

t_memory *remove_memory(int mem_number) {
	bool isMem(void *mem) {
		return ((t_memory*) mem)->mid == mem_number;
	}
	sem_wait(&MUTEX_MEMORIES);
	t_memory *m = list_remove_by_condition(memories, isMem);
	sem_post(&MUTEX_MEMORIES);
	return m;
}

void add_memory_to_cons_type(int mem_number, e_cons_type consType) {
	bool isMem(void *mem) {
		return ((t_memory*) mem)->mid == mem_number;
	}
	sem_wait(&MUTEX_MEMORIES);
	t_memory *selectedMem = list_find(memories, isMem);
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
	t_memory *m = NULL;
	sem_wait(&MUTEX_MEMORIES);
	if(list_size(memories) > 0)
		m = list_get(memories, random() % list_size(memories));
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

void update_sc() {
	bool isSC(void *t) {
		return ((t_table*)t)->consType == CONS_SC;
	}
	void updateMemories(void *t) {
		add_memories_to_table((t_table*)t);
	}
	sem_wait(&MUTEX_TABLES);
	t_list *scTables = list_filter(tables, isSC);
	list_iterate(scTables, updateMemories);
	sem_post(&MUTEX_TABLES);
	list_destroy(scTables);
}

void remove_sc(int mid) {
	void removeMem(void *e) {
		t_table *t = (t_table*)e;
		if((int)list_get(t->memories, 0) == mid) {
			list_remove(t->memories, 0);
		}
	}
	sem_wait(&MUTEX_TABLES);
	list_iterate(tables, removeMem);
	sem_post(&MUTEX_TABLES);
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
			if(list_size(mems) > 0 && list_size(t->memories) == 0) {
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

void *refreshMetadata() {
	while(!exitFlag) {
		int i = 0;
		for(i = 0; i < 10; i++) {
			usleep(get_metadata_refresh_rate() * 100);
			if(exitFlag) break;
		}

		if(!exitFlag) {
			log_info(logger, " >> Actualizando metadata de tablas.");
			int status = qDescribe(NULL, logger);
			if(status == 1) {
				log_info(logger, " >> Metadata actualizada correctamente.");
			} else {
				log_info(logger, " >> No se pudo actualizar la metadata las tablas.");
			}
		}

	}
	return NULL;
}

void journal(){
	void journalMem(void *m) {
			qJournal((t_memory*)m, logger);
		}
		list_iterate(memories,journalMem);
}

void *metrics() {
	printf("\nReads : Read Latency/30s = 0 : 0ms\nWrites : WritesLatency/30s = 0 : 0ms\nMemory Load = \n\n");

	while(!exitFlag) {

		int i = 0;
		for(i = 0; i < 10; i++) {
			usleep(3 * 1000 * 1000);
			if(exitFlag) break;
		}

		log_metrics();
		delete_old_ops();
	}

	return NULL;
}

void log_metrics() {
	uint64_t readLatency = 0, writeLatency = 0;

	int reads = get_reads();
	uint64_t readsTime = get_readsTime();
	int writes = get_writes();
	uint64_t writesTime = get_writesTime();

	if(reads != 0) {
		readLatency = readsTime/reads;
	}
	if(writes != 0) {
		writeLatency = writesTime/writes;
	}

	log_info(logger, " >> Metricas >>");
	log_info(logger, "Reads : Read Latency/30s = %d : %llums", reads, readLatency);
	log_info(logger, "Writes : Writes Latency/30s = %d : %llums", writes, writeLatency);
	log_info(logger, " << Fin Metricas <<");
}

void update_screen() {
	void print_m_load(void *mem) {
		t_memory *m = (t_memory*)mem;
		int mLoad = (m->totalOperations * 100) / totalOperations;
		printf("%d : %d%% ", m->mid, mLoad);
	}

	uint64_t readLatency = 0, writeLatency = 0;

	int reads = get_reads();
	uint64_t readsTime = get_readsTime();
	int writes = get_writes();
	uint64_t writesTime = get_writesTime();

	log_info(logger, "%d %llu %d %llu", reads, readsTime, writes, writesTime);

	if(reads != 0) {
		readLatency = readsTime/reads;
	}
	if(writes != 0) {
		writeLatency = writesTime/writes;
	}

	system("clear");
	printf("<LFS Kernel>\n\n");
	display_memories();

	printf("\nReads : Read Latency/30s = %d : %llums\nWrites : Write Latency/30s = %d : %llums\nMemory Load = ", get_reads(), readLatency, get_writes(), writeLatency);

	sem_wait(&MUTEX_TOTALOPS);
	if(totalOperations != 0) {
		sem_wait(&MUTEX_MEMORIES);
		list_iterate(memories, print_m_load);
		sem_post(&MUTEX_MEMORIES);
	}
	sem_post(&MUTEX_TOTALOPS);
	printf("\n\n\n");
}

void metrics_new_select(uint64_t start, uint64_t end) {
	operation_t *op = new_operation(OP_READ, end - start);
	sem_wait(&MUTEX_READS);
	list_add(reads, op);
	sem_post(&MUTEX_READS);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

void metrics_new_insert(uint64_t start, uint64_t end) {
	operation_t *op = new_operation(OP_READ, end - start);
	sem_wait(&MUTEX_WRITES);
	list_add(writes, op);
	sem_post(&MUTEX_WRITES);
	sem_wait(&MUTEX_TOTALOPS);
	totalOperations++;
	sem_post(&MUTEX_TOTALOPS);
}

void delete_old_ops() {
	bool isOldOp(void *op) {
		return !op_is_recent(op);
	}
	sem_wait(&MUTEX_READS);
	list_remove_and_destroy_by_condition(reads, isOldOp, operation_destroy);
	sem_post(&MUTEX_READS);
	sem_wait(&MUTEX_WRITES);
	list_remove_and_destroy_by_condition(writes, isOldOp, operation_destroy);
	sem_post(&MUTEX_WRITES);
}

int get_reads() {
	sem_wait(&MUTEX_READS);
	t_list *last_reads = list_filter(reads, op_is_recent);
	int q = list_size(last_reads);
	list_destroy(last_reads);
	sem_post(&MUTEX_READS);
	return q;
}

uint64_t get_readsTime() {
	uint64_t q = 0;
	void sumReadTime(void *op) {
		q += ((operation_t*)op)->duration;
	}
	sem_wait(&MUTEX_READS);
	t_list *last_reads = list_filter(reads, op_is_recent);
	list_iterate(last_reads, sumReadTime);
	list_destroy(last_reads);
	sem_post(&MUTEX_READS);
	return q;
}

int get_writes() {
	sem_wait(&MUTEX_WRITES);
	t_list *last_writes = list_filter(writes, op_is_recent);
	int q = list_size(last_writes);
	list_destroy(last_writes);
	sem_post(&MUTEX_WRITES);
	return q;
}

uint64_t get_writesTime() {
	uint64_t q = 0;
	void sumWriteTime(void *op) {
		q += ((operation_t*)op)->duration;
	}
	sem_wait(&MUTEX_WRITES);
	t_list *last_writes = list_filter(writes, op_is_recent);
	list_iterate(last_writes, sumWriteTime);
	list_destroy(last_writes);
	sem_post(&MUTEX_WRITES);
	return q;
}

char *get_memory_ip() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	char *ip = string_duplicate(config_get_string_value(config, "MEM_IP"));
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return ip;
}

char *get_memory_port() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	char *port = string_duplicate(config_get_string_value(config, "MEM_PORT"));
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return port;
}

int get_quantum() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	int q = config_get_int_value(config, "QUANTUM");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return q;
}

int get_multiprogramming_degree() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	int md = config_get_int_value(config, "MULT_DEGREE");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return md;
}

int get_metadata_refresh_rate() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	int mdr = config_get_int_value(config, "MD_REFRESH_RATE");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return mdr;
}

int get_execution_delay() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	int ed = config_get_int_value(config, "EXEC_DELAY");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return ed;
}

int get_gossip_delay() {
	sem_wait(&MUTEX_CONFIG);
	load_config();
	int gd = config_get_int_value(config, "GOSSIP_DELAY");
	config_destroy(config);
	sem_post(&MUTEX_CONFIG);
	return gd;
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

uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}
