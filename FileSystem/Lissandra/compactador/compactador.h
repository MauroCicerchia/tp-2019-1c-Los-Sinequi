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
#include"../../Fs/fs.h"

extern t_log *logger;

typedef struct{
	char *name;
	int parts;
	int ctime;
	sem_t MUTEX_TABLE_PART;
	sem_t MUTEX_DROP_TABLE;
}activeTable;


void compact(activeTable *table);
void com_compactTmpsC(t_list *tmpsC,char *tableUrl, activeTable *table);
bool biggerTimeStamp(void *elem1, void *elem2);
uint64_t com_timestamp(void *insert);
uint16_t com_key(char *insert);
t_list *com_changeTmpsExtension(t_list *tmps, char *tableUrl);
t_list *com_getAllKeys(t_list *inserts);
bool keyIsAdded(uint16_t key,t_list *keys);
void com_saveInPartition(t_list *keys,t_list *allInserts, activeTable *table);
bool com_gotKey(uint16_t key, char *insert);
