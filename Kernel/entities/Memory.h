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
	int totalOperations;
} t_memory;

t_memory *memory_create(int, char*, char*);
void memory_destroy(void*);
void memory_add_cons_type(t_memory*, e_cons_type);
void memory_remove_cons_type(t_memory*, e_cons_type);
int memory_is_cons_type(t_memory*, e_cons_type);
bool memory_is_sc(void*);
bool memory_is_shc(void*);
bool memory_is_ec(void*);

#endif
