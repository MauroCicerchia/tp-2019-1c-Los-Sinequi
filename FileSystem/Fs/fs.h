/* FS_H */
#ifndef FS_H
#define FS_H

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stddef.h>
#include<dirent.h>

#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/txt.h>
#include<commons/config.h>


#include<sys/types.h>
#include<sys/stat.h>

#include"blocks.h"
#include"../Lissandra/memtable/memtable.h"
#include"../Lissandra/compactador/activeTable.h"

extern char* absoluto;
extern t_log *logger;
extern t_config *FsMetadataCfg;
extern int tmpNo;
extern t_list *sysTables;
extern pthread_mutex_t MUTEX_LISTACTIVETABLES;

typedef struct{
	char *consistency;
	char *partitions;
	char *ctime;
}metadata;

int fs_tableExists(char*);

int fs_create(char*,char*,int,int);

char *makeUrlForPartition(char *,char *);
char *makeTableUrl(char *);
void makeFiles(char *,int);
void makeDirectories(char*);
void makeMetadataFile(char *);

void loadMetadata(char *,char *,int ,int);
t_config *load_metadataConfig(char*);

char *getConsistency(t_config*);
char *getCTime(t_config*);
char *getPartitions(t_config*);

char *fs_getBitmapUrl();
char *fs_getlfsMetadataUrl();
char* fs_getBlocksUrl();

t_list *getAllTmps(char *);
void fs_setActualTmps();
void incrementTmpNo(t_list *alltmps);

void fs_toDump(char*,char*); //nombre de tabla y el string a dumpear

metadata *fs_getTableMetadata(char*);

t_list *fs_getListOfInserts(char*,int);

void fs_createBlocks(int);

t_list *fs_getAllTables();

void fs_cleanTmpsC(char *tableUrl);
bool isTmpc(char *string);

void fs_setActiveTables();
activeTable *getActiveTable(char *);
#endif /* FS_H */
