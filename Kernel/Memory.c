#include"Memory.h"

t_memory *memory_create(char *ip, int port) {
	t_memory *memory = (t_memory*) malloc(sizeof(t_memory));
	memory->ip = ip;
	memory->port = port;
	memory->consType = CONS_UNDEFINED;
	return memory;
}

void memory_destroy(void *memory) {
	free(((t_memory*) memory)->ip);
	free(memory);
}

void memory_set_cons_type(t_memory *memory, e_cons_type consType) {
	memory->consType = consType;
}
