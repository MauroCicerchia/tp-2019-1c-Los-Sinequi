#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<semaphore.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/txt.h>
#include"../memtable/memtable.h"
#include"../../Fs/fs.h"
#include"../dump/dump.h"

#include"../delayer/delayer.h"

extern t_log *logger;
extern pthread_mutex_t MUTEX_MEMTABLE;
extern int valueSize;

int qinsert(char*, char*, char*, char*);
char* toLFSmode(char*,char*,char*);
