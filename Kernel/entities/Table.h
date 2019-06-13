#include<stdio.h>
#include<stdlib.h>
#include<sharedLib/consistency.h>

#ifndef TABLE_H
#define TABLE_H

typedef struct {
	char* name;
	e_cons_type consType;
	int partitions;
	int compTime;
} t_table;

t_table *table_create();
void table_destroy(void*);

#endif
