#include"Table.h"

t_table *table_create(char *name, e_cons_type consType, int part, int compTime) {
	void *memoryToID(void *memory) {
		return (void*)((t_memory*)memory)->mid;
	}
	t_table *t = (t_table*)malloc(sizeof(t_table));
	t->name = string_duplicate(name);
	t->consType = consType;
	t->partitions = part;
	t->compTime = compTime;
	t->memories = list_create();
	add_memories_to_table(t);
	return t;
}

void table_add_memory_by_id(t_table* table, int mid) {
	list_add(table->memories, (void*)mid);
}

void table_destroy(void *table) {
	free(((t_table*)table)->name);
	list_destroy_and_destroy_elements(((t_table*)table)->memories, free);
	free(table);
}
