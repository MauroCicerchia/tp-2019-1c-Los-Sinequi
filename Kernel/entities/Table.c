#include"Table.h"

t_table *table_create(char *name, e_cons_type consType, int part, int compTime) {
	t_table *t = (t_table*)malloc(sizeof(t_table));
	t->name = name;
	t->consType = consType;
	t->partitions = part;
	t->compTime = compTime;
	return t;
}

void table_destroy(void *table) {
	free(((t_table*)table)->name);
	free(table);
}
