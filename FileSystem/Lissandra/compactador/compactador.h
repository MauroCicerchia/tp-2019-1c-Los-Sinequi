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
#include"../compactador/activeTable.h"

extern t_log *logger;
extern int exitFlag;

#ifndef COMPACTADOR_H
#define COMPACTADOR_H

void compact(activeTable *table);
void com_compactTmpsC(t_list *tmpsC,char *tableUrl, activeTable *table);
bool com_biggerTimeStamp(void *elem1, void *elem2);
uint64_t com_timestamp(void *insert);
char *com_key(char *insert);
t_list *com_changeTmpsExtension(t_list *tmps, char *tableUrl);
t_list *com_getAllKeys(t_list *inserts);
bool keyIsAdded(char *key,t_list *keys);
void com_saveInPartition(t_list *keys,t_list *allInserts, activeTable *table);
bool com_gotKey(char *key, char *insert);

void *threadCompact(char *tableName);
activeTable *com_getActiveTable(char *tableName);
bool tableIsActive(char *tableName);
int com_getCTime(char *tableName);

#endif
