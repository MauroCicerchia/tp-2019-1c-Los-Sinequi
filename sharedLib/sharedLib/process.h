#include<stdlib.h>
#include<commons/collections/queue.h>
#include"query.h"

#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
	int pid;
	int pc;
	t_list *querys;
} t_process;

/*
 * Crea un proceso nuevo y le asigna una lista de querys.
 */
t_process *process_create(t_list*);

/*
 * Destruye un proceso y su lista de querys.
 */
void process_destroy(void*);

#endif
