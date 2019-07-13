#ifndef ACTIVETABLE_H
#define ACTIVETABLE_H

typedef struct{
	char *name;
	int parts;
	int ctime;
	pthread_mutex_t MUTEX_TABLE_PART;
	pthread_mutex_t MUTEX_DROP_TABLE;
}activeTable;

#endif
