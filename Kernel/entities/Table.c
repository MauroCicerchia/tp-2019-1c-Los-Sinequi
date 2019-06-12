#include"Table.h"

t_table *table_create() {
	return (t_table*)malloc(sizeof(t_table));
}

void table_destroy(void *table) {
	free(((t_table*)table)->name);
	free(table);
}

