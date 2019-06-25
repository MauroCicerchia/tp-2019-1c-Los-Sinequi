#ifndef ACTIVETABLE_H
#define ACTIVETABLE_H

typedef struct{
	char *name;
	int parts;
	int ctime;
	sem_t MUTEX_TABLE_PART;
	sem_t MUTEX_DROP_TABLE;
}activeTable;

#endif
