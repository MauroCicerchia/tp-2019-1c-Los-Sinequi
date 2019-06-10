#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>

#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/txt.h>
#include<commons/config.h>

#include<sys/types.h>
#include<sys/stat.h>

#include"blocks.h"


extern char* absoluto;
extern t_log *logger;
extern t_config *metadataCfg;
extern int tmpNo;

/* FS_H */
#ifndef FS_H
#define FS_H

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
void *load_metadataConfig(char*);

char *getConsistency();
char *getCTime();
char *getPartitions();

char *fs_getBitmapUrl();
char *fs_getlfsMetadataUrl();
char* fs_getBlocksUrl();

void fs_toDump(char*,char*); //nombre de tabla y el string a dumpear

metadata *fs_getTableMetadata(char*);

//t_list *fs_getListOfInserts(char*);

void fs_createBlocks(int);

#endif /* FS_H */
