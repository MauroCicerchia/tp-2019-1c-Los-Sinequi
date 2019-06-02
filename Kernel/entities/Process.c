#include"process.h"

t_process *process_create(t_list *querys) {
	t_process *newProcess = (t_process*) malloc(sizeof(t_process));
	newProcess->querys = querys;
	return newProcess;
}

void process_destroy(void *process) {
	list_destroy_and_destroy_elements(((t_process*)process)->querys, query_destroy);
	free(process);
}

int process_length(t_process *process) {
	return list_size(process->querys);
}
