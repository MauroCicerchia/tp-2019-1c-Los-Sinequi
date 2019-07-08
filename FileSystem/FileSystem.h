#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<string.h>
#include<commons/log.h>
#include<pthread.h>
#include<semaphore.h>
#include<commons/config.h>
//#include<readline/readline.h>
//#include<sharedLib/console.h>
//#include<sharedLib/server.h>
#include"Lissandra/api/api.h"
#include"sys/inotify.h"
#include"Fs/bitarray.h"
#include"Fs/fs.h"

#include"Lissandra/connection/connection.h"

#include"Lissandra/compactador/compactador.h"

void init_FileSystem();
void kill_FileSystem();
void iniciar_logger(t_log **logger);
int get_dump_time();
int get_retard_time();
char *get_port();
char *get_ip();
int get_valueSize();
int get_size_of_blocks(t_config *metadata);
int get_blocks_cuantityy(t_config *metadata);
char *get_fs_route();
void *threadConfigModify();
void *threadListenToClient();
void *threadDump();
void load_config();
t_config *load_lfsMetadata();
void threadForCompact(char *tableName);

