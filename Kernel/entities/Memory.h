#include<stdio.h>
#include<stdlib.h>
#include<sharedLib/consistency.h>

#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
	int mid;
	char *ip;
	char *port;
	int consTypes[3];
} t_memory;

t_memory *memory_create(char*, char*);
void memory_destroy(void*);
void memory_add_cons_type(t_memory*, e_cons_type);
void memory_remove_cons_type(t_memory*, e_cons_type);
int memory_is_cons_type(t_memory*, e_cons_type);

#endif
