#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>
#include<sharedLib/consistency.h>
#include"Memory.h"

#ifndef TABLE_H
#define TABLE_H

typedef struct {
	char* name;
	e_cons_type consType;
	int partitions;
	int compTime;
	t_list *memories;
} t_table;

t_table *table_create();
void table_add_memory_by_id(t_table*, int);
void table_destroy(void*);

#endif
