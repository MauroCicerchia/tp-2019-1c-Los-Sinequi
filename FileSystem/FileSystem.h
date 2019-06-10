//#include<stdio.h>
//#include<stdlib.h>
//#include<string.h>
//#include<commons/log.h>
#include<pthread.h>
#include<semaphore.h>
#include<commons/config.h>
//#include<readline/readline.h>
//#include<sharedLib/console.h>
#include<sharedLib/server.h>
#include"Lissandra/api/api.h"
#include"sys/inotify.h"
#include"./Fs/bitarray.h"


void init_FileSystem();
void kill_FileSystem();
void iniciar_logger(t_log **logger);
int get_dump_time();
int get_retard_time();
char *get_fs_route();
void *threadConfigModify();
void *threadDump();
void load_config();

