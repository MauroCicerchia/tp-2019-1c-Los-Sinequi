#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<commons/collections/list.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/client.h>
#include<sharedLib/process.h>

#define QUANTUM = 4;
#define MULT_DEGREE = 3;

e_query processQuery(char *, t_log*);
int read_lql_file(char*);
void add_process_to_new(t_process*);

void iniciar_logger(t_log **logger)
{
	*logger = log_create("Kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
}

int iniciar_cliente() {
	return connectToServer();
//	sendMessages(server);
//	closeConnection(server);
}
