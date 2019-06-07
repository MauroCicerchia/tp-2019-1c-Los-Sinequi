#include "Memory.h"

t_memory *memory_create(char *ip, char *port) {
	t_memory *memory = (t_memory*) malloc(sizeof(t_memory));
	memory->ip = ip;
	memory->port = port;
	memory->consTypes[0] = 0;
	memory->consTypes[1] = 0;
	memory->consTypes[2] = 0;
	return memory;
}

void memory_destroy(void *memory) {
//	free(((t_memory*) memory)->ip);
	free(memory);
}

void memory_add_cons_type(t_memory *memory, e_cons_type consType) {
	switch(consType) {
		case CONS_SC: memory->consTypes[0] = 1; break;
		case CONS_SHC: memory->consTypes[1] = 1; break;
		case CONS_EC: memory->consTypes[2] = 1; break;
		default: break;
	}
}

void memory_remove_cons_type(t_memory *memory, e_cons_type consType) {
	switch(consType) {
		case CONS_SC: memory->consTypes[0] = 0; break;
		case CONS_SHC: memory->consTypes[1] = 0; break;
		case CONS_EC: memory->consTypes[2] = 0; break;
		default: break;
	}
}

int memory_is_cons_type(t_memory *memory, e_cons_type consType) {
	switch(consType) {
		case CONS_SC: return memory->consTypes[0]; break;
		case CONS_SHC: return memory->consTypes[1]; break;
		case CONS_EC: return memory->consTypes[2];; break;
		default: return 0; break;
	}
}
