/* FS_H */
#ifndef FS_H
#define FS_H

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stddef.h>

#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/txt.h>
#include<commons/config.h>


#include<sys/types.h>
#include<sys/stat.h>

#include"blocks.h"
#include"../Lissandra/memtable/memtable.h"

extern char* absoluto;
extern t_log *logger;
extern t_config *FsMetadataCfg;
extern int tmpNo;

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

char **getAllTmps(char *);

void fs_toDump(char*,char*); //nombre de tabla y el string a dumpear

metadata *fs_getTableMetadata(char*);

t_list *fs_getListOfInserts(char*,int);

void fs_createBlocks(int);

#endif /* FS_H */
