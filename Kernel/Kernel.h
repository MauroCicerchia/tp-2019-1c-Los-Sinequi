#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<commons/collections/list.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/client.h>
#include"Memory.h"
#include"Table.h"

void load_logger();
void load_config();
void init_kernel();
void kill_kernel();
e_query newQuery(char *);
int read_lql_file(char*);
void add_process_to_new(t_process*);
void *new_to_ready();
void setConfigParameter(char*);
void init_memory();
int connect_to_memory();
void request_memory_pool(int);
void display_memories();
void add_memory_to_cons_type(int, e_cons_type);
t_memory *get_sc_memory();
char *get_memory_ip();
int get_memory_port();
int get_quantum();
int get_multiprogramming_degree();
int get_metadata_refresh_rate();
int get_execution_delay();

int iniciar_cliente() {
	return connectToServer();
//	sendMessages(server);
//	closeConnection(server);
}
