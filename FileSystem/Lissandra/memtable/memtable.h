#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<semaphore.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/txt.h>

extern t_list *memtable;
extern t_log *logger;
extern pthread_mutex_t MUTEX_MEMTABLE;

#ifndef MEMTABLE_H
#define MEMTABLE_H

typedef struct{
	char *timestamp;
	char *key;
	char *value;
}Iinsert;

typedef struct{
	char *table;
	t_list *inserts;
}Itable;


void mt_insert(char *, char*, char *,char *);
bool mt_tableExists(char*);
t_list *mt_getTableToInsert(char*);
void mt_addNewInsert(t_list*, char*, char*, char*);
void mt_clean();
void mt_cleanPivot(t_list *);
void tableDestroyer(void*);
void insertDestroyer(void*);
void mt_getListofInserts(char *table,t_list *list);
#endif
