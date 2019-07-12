#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<sharedLib/console.h>
#include<sharedLib/client.h>
#include<sharedLib/query.h>
#include<sharedLib/packaging.h>
#include<sharedLib/consistency.h>
#include<semaphore.h>

typedef struct{
	char* memory_ip;
	char* memory_port;
}memory;

t_list* gossip_table;

void delete_mem_from_gossip_table(char*,char*,t_log*);
void execute_gossip_client(t_log*,char*,sem_t);
void execute_gossip_server(int,t_log*,sem_t);
void add_to_gossip_table(char*, char*,t_log*);
memory* memory_create(char*,char*);
void gossip_table_destroy();
void memory_destroy(void*);
void recv_gossip_table(int,t_log*);
void send_gossip_table(int);
