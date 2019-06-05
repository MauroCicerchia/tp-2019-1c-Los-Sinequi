#include "Process.h"

t_process *process_create(int pid, t_list *querys) {
	t_process *newProcess = (t_process*) malloc(sizeof(t_process));
	newProcess->pc = 0;
	newProcess->pid = pid;
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

t_query *process_next_query(t_process *process) {
	process->pc++;
	return list_get(process->querys, process->pc - 1);
}

int process_finished(t_process *process) {
	return process->pc >= process_length(process);
}
