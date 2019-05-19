#include<stdio.h>
#include<sharedLib/consistency.h>

typedef struct {
	int mid;
	char *ip;
	int port;
	e_cons_type consType;
} t_memory;

t_memory *memory_create(char*, int);
void memory_destroy(void*);
void memory_set_cons_type(t_memory*, e_cons_type);
