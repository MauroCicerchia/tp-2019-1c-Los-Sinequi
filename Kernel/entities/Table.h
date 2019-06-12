#include<stdio.h>
#include<stdlib.h>
#include<sharedLib/consistency.h>

typedef struct {
	char* name;
	e_cons_type consType;
	int partitions;
	int compTime;
} t_table;

t_table *table_create();
void table_destroy(void*);

