#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<sharedLib/query.h>
#include"../../Fs/fs.h"
#include"../../Lissandra/compactador/activeTable.h"
#include"../../Lissandra/compactador/compactador.h"

#include"../delayer/delayer.h"

extern t_log *logger;

int qcreate(char*, char*, char* , char*);

activeTable *addToActiveTables(char *table, int parts, int ctime);
