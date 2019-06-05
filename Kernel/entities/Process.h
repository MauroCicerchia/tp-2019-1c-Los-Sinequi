#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/queue.h>
#include<sharedLib/query.h>

typedef struct {
	int pid;
	int pc;
	t_list *querys;
} t_process;

/*
 * Crea un proceso nuevo y le asigna una lista de querys.
 */
t_process *process_create(int, t_list*);

/*
 * Destruye un proceso y su lista de querys.
 */
void process_destroy(void*);

/*
 * Devuelve la cantidad de querys de un proceso.
 */
int process_length(t_process*);

/*
 * Devuelve la siguiente query e incrementa el pc. Si no hay query devuelve null.
 */
t_query *process_next_query(t_process*);

/*
 * Devuelve 1 si el proceso termino y 0 en caso contrario.
 */
int process_finished(t_process*);
